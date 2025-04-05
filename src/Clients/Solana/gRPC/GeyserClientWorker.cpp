// SPDX-License-Identifier: AGPL-3.0-or-later
/*
 * Copyright (C) 2025 to1dev <https://arc20.me/to1dev>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "GeyserClientWorker.h"

namespace Daitengu::Clients::Solana::gRPC {

GeyserClientWorker::GeyserClientWorker(QString address,
    std::vector<std::shared_ptr<TransactionFilter>> filters,
    StorageManager* storage, NotificationManager* notifier,
    FilterManager* filterMgr, QObject* parent)
    : QObject(parent)
    , address_(std::move(address))
    , initialFilters_(std::move(filters))
    , storageManager_(storage)
    , notificationManager_(notifier)
    , filterManager_(filterMgr)
    , cq_(std::make_unique<grpc::CompletionQueue>())
{
    for (auto& filter : initialFilters_) {
        filterManager_->addFilter("initial_"
                + std::to_string(reinterpret_cast<uintptr_t>(filter.get())),
            filter);
    }
}

GeyserClientWorker::~GeyserClientWorker()
{
    stop();
}

QCoro::Task<void> GeyserClientWorker::start()
{
    shouldRun_ = true;
    co_await runConnection();
}

QCoro::Task<bool> GeyserClientWorker::asyncRead(
    std::unique_ptr<grpc::ClientAsyncReaderWriter<geyser::SubscribeRequest,
        geyser::SubscribeUpdate>>& stream,
    geyser::SubscribeUpdate& update)
{
    struct ReadTag {
        geyser::SubscribeUpdate* update;
        bool* ok;
    };

    bool ok = false;
    ReadTag tag { &update, &ok };
    stream->Read(&update, &tag);

    void* got_tag;
    bool completed = cq_->Next(&got_tag, &ok);
    if (!completed || got_tag != &tag) {
        spdlog::warn(
            "Async read failed or timed out for {}", address_.toStdString());
        co_return false;
    }

    co_return ok;
}

QCoro::Task<void> GeyserClientWorker::runConnection()
{
    constexpr size_t BATCH_SIZE = 100;
    std::vector<std::pair<std::string, std::string>> batch;
    int retryDelay = 1;
    constexpr int MAX_DELAY = 60;

    while (shouldRun_) {
        bool shouldRetry = false;
        try {
            grpc::ChannelArguments args;
            args.SetInt("grpc.max_receive_message_length", 64 * 1024 * 1024);
            args.SetInt("grpc.keepalive_time_ms", 30000);
            args.SetInt("grpc.keepalive_timeout_ms", 10000);
            args.SetInt("grpc.keepalive_permit_without_calls", 1);
            args.SetInt("grpc.http2.max_pings_without_data", 0);
            args.SetInt("grpc.http2.min_time_between_pings_ms", 30000);

            auto channel = grpc::CreateCustomChannel(address_.toStdString(),
                grpc::SslCredentials(grpc::SslCredentialsOptions()), args);

            auto deadline
                = std::chrono::system_clock::now() + std::chrono::seconds(5);

            if (!channel->WaitForConnected(deadline)) {
                spdlog::error("WaitForConnected failed for {}, retrying in {}s",
                    address_.toStdString(), retryDelay);
                co_await QCoro::sleepFor(std::chrono::seconds(retryDelay));
                retryDelay = std::min(retryDelay * 2, MAX_DELAY);
                continue;
            }

            auto stub = geyser::Geyser::NewStub(channel);
            grpc::ClientContext context;
            grpc::Status status;
            auto stream = stub->AsyncSubscribe(&context, cq_.get(), this);

            struct WriteTag {
                bool* ok;
            };

            bool writeOk = false;
            WriteTag writeTag { &writeOk };

            geyser::SubscribeRequest req;
            req.set_commitment(geyser::CommitmentLevel::PROCESSED);

            // for test
            auto* txs_map = req.mutable_transactions();
            geyser::SubscribeRequestFilterTransactions tx_filters;
            tx_filters.set_vote(false);
            tx_filters.set_failed(false);
            (*txs_map)["tokens"] = tx_filters;

            stream->StartCall(&writeTag);
            void* got_tag;
            bool completed = cq_->Next(&got_tag, &writeOk);
            if (!completed || got_tag != &writeTag) {
                spdlog::error(
                    "Async StartCall failed for {}", address_.toStdString());
                continue;
            }

            stream->Write(req, &writeTag);
            completed = cq_->Next(&got_tag, &writeOk);
            if (!completed || got_tag != &writeTag || !writeOk) {
                spdlog::error(
                    "Async write failed for {}", address_.toStdString());
                continue;
            }

            geyser::SubscribeUpdate update;
            while (shouldRun_) {
                bool readOk = co_await asyncRead(stream, update);
                if (!readOk)
                    break;

                if (update.update_oneof_case()
                    == geyser::SubscribeUpdate::kTransaction) {
                    auto sig = update.transaction().transaction().signature();
                    batch.emplace_back(sig, update.SerializeAsString());
                    if (batch.size() >= BATCH_SIZE) {
                        co_await processBatch(batch);
                        batch.clear();
                    }
                }
            }
            if (!batch.empty())
                co_await processBatch(batch);

            if (!shouldRun_)
                context.TryCancel();

            stream->Finish(&status, &writeTag);
            cq_->Next(&got_tag, &completed);
            if (!status.ok())
                spdlog::error("Stream error for {}: {}", address_.toStdString(),
                    status.error_message());

            retryDelay = 1;
        } catch (const std::exception& e) {
            spdlog::error("Exception in {}: {}, retrying in {}s",
                address_.toStdString(), e.what(), retryDelay);
            shouldRetry = true;
            retryDelay = std::min(retryDelay * 2, MAX_DELAY);
        }
        if (shouldRetry) {
            co_await QCoro::sleepFor(std::chrono::seconds(retryDelay));
        }
    }

    co_return;
}

QCoro::Task<void> GeyserClientWorker::processBatch(
    std::vector<std::pair<std::string, std::string>>& batch)
{
    size_t hits = 0;
    for (const auto& [sig, data] : batch) {
        geyser::SubscribeUpdate update;
        update.ParseFromString(data);
        hits += co_await filterManager_->processTransaction(
            update.transaction());
    }
    co_await storageManager_->storeBatch(batch);
    co_await notificationManager_->sendBatchNotifications(
        QString("Processed %1 transactions").arg(batch.size()));
    Q_EMIT transactionProcessed(hits);
    Q_EMIT updateReceived(QString("Batch processed: %1").arg(batch.size()));

    co_return;
}

void GeyserClientWorker::stop()
{
    shouldRun_ = false;
    cq_->Shutdown();
}
}
