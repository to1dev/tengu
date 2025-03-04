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

namespace Daitengu::Clients::Solana {

GeyserClientWorker::GeyserClientWorker(const QString& address,
    std::vector<std::shared_ptr<TransactionFilter>> filters, QObject* parent)
    : QObject(parent)
    , address_(address)
    , shouldRun_(false)
    , filters_(std::move(filters))
{
}

void GeyserClientWorker::start()
{
    shouldRun_ = true;
    runConnection();
}

void GeyserClientWorker::stop()
{
    shouldRun_ = false;
    cancelCurrentContext();
}

void GeyserClientWorker::runConnection()
{
    while (shouldRun_) {
        try {
            grpc::ChannelArguments args;
            args.SetInt("grpc.max_receive_message_length", 64 * 1024 * 1024);
            args.SetInt("grpc.keepalive_time_ms", 30000);
            args.SetInt("grpc.keepalive_timeout_ms", 10000);
            args.SetInt("grpc.keepalive_permit_without_calls", 1);
            args.SetInt("grpc.http2.max_pings_without_data", 0);
            args.SetInt("grpc.http2.min_time_between_pings_ms", 30000);

            auto creds = grpc::SslCredentials(grpc::SslCredentialsOptions());
            auto channel = grpc::CreateCustomChannel(
                address_.toStdString(), creds, args);

            auto deadline
                = std::chrono::system_clock::now() + std::chrono::seconds(5);
            if (!channel->WaitForConnected(deadline)) {
                Q_EMIT logMessage("[GeyserClientWorker] WaitForConnected() "
                                  "failed, retry in 5s...");
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue;
            }

            std::unique_ptr<geyser::Geyser::Stub> stub_stream
                = geyser::Geyser::NewStub(channel);

            grpc::ClientContext context;
            {
                QMutexLocker locker(&contextMutex_);
                context_ = &context;
            }
            std::unique_ptr<grpc::ClientReaderWriter<geyser::SubscribeRequest,
                geyser::SubscribeUpdate>>
                stream = stub_stream->Subscribe(&context);

            geyser::SubscribeRequest req;
            req.set_commitment(geyser::CommitmentLevel::PROCESSED);

            auto* txs_map = req.mutable_transactions();
            geyser::SubscribeRequestFilterTransactions tx_filters;
            tx_filters.set_vote(false);
            tx_filters.set_failed(false);
            (*txs_map)["tokens"] = tx_filters;

            if (!stream->Write(req)) {
                Q_EMIT logMessage(
                    "[GeyserClientWorker] SubscribeRequest Write() failed");
                continue;
            }

            geyser::SubscribeUpdate update;
            while (shouldRun_ && stream->Read(&update)) {
                handleUpdate(update, stream.get());
            }

            if (!shouldRun_) {
                context.TryCancel();
            }

            auto status = stream->Finish();
            if (!status.ok()) {
                Q_EMIT logMessage(QString(
                    "[GeyserClientWorker] Stream finished with error: %1")
                        .arg(QString::fromStdString(status.error_message())));
            } else {
                Q_EMIT logMessage(
                    "[GeyserClientWorker] Stream finished normally.");
            }

            {
                QMutexLocker locker(&contextMutex_);
                context_ = nullptr;
            }
        } catch (const std::exception& e) {
            std::cerr << "[GeyserClientWorker] Exception: " << e.what()
                      << std::endl;
        }

        if (shouldRun_) {
            Q_EMIT logMessage("[GeyserClientWorker] Reconnecting in 5s...");
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
    Q_EMIT finished();
}

void GeyserClientWorker::cancelCurrentContext()
{
    QMutexLocker locker(&contextMutex_);
    if (context_) {
        context_->TryCancel();
    }
}

void GeyserClientWorker::handleUpdate(const geyser::SubscribeUpdate& update,
    grpc::ClientReaderWriter<geyser::SubscribeRequest, geyser::SubscribeUpdate>*
        stream)
{
    switch (update.update_oneof_case()) {
    case geyser::SubscribeUpdate::kTransaction: {
        for (auto& filter : filters_) {
            filter->processTransaction(update.transaction());
        }
        break;
    }
    case geyser::SubscribeUpdate::kPing: {
        geyser::SubscribeRequest pong_req;
        pong_req.mutable_ping()->set_id(1);
        if (!stream->Write(pong_req)) {
            std::cerr << "[GeyserClientWorker] Pong response failed.\n";
        }
        break;
    }

    case geyser::SubscribeUpdate::kAccount: {
        Q_EMIT updateReceived("Account Update arrived.");
        break;
    }

    case geyser::SubscribeUpdate::kSlot: {
        Q_EMIT updateReceived("Slot Update arrived.");
        break;
    }

    case geyser::SubscribeUpdate::kPong: {
        std::cout << "[GeyserClientWorker] Received Pong id: "
                  << update.pong().id() << std::endl;
        break;
    }

    default:
        break;
    }
}

}
