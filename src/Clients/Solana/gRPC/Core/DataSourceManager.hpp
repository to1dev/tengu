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

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <QObject>
#include <QTimer>

#include <grpcpp/grpcpp.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "ConfigManager.hpp"
#include "FilterManager.hpp"
#include "NotificationManager.hpp"
#include "StorageManager.hpp"

#include "../Utils/Logger.hpp"

namespace solana {

class GeyserClientWorker : public QObject {
    Q_OBJECT

public:
    GeyserClientWorker(const std::string& sourceId, const std::string& address,
        StorageManager& storage, NotificationManager& notifier,
        FilterManager& filter, QObject* parent = nullptr)
        : QObject(parent)
        , sourceId_(sourceId)
        , address_(address)
        , storage_(storage)
        , notification_(notifier)
        , filter_(filter)
    {
        start();
    }

    ~GeyserClientWorker()
    {
        stop();
    }

    bool isConnected() const
    {
        return connected_;
    }

    uint64_t getTotalTransactions() const
    {
        return totalTransactions_;
    }

    uint64_t getProcessedBatches() const
    {
        return processedBatches_;
    }

    std::string getAddress() const
    {
        return address_;
    }

Q_SIGNALS:
    void dataReceived(const std::string& sourceId, const nlohmann::json& data);
    void error(const QString& message);

private:
    struct AsyncCall {
        geyser::SubscribeUpdate response;
        std::unique_ptr<grpc::ClientAsyncReaderWriter<geyser::SubscribeRequest,
            geyser::SubscribeUpdate>>
            reader;
        grpc::ClientContext context;
        grpc::Status status;
        enum class State { START, WRITE, READ, FINISHED };
        State state = State::START;
    };

    void start()
    {
        worker_ = std::jthread([this](std::stop_token stoken) {
            try {
                cq_ = std::make_unique<grpc::CompletionQueue>();
                channel_ = grpc::CreateCustomChannel(address_,
                    grpc::SslCredentials({}), createChannelArguments());
                stub_ = geyser::Geyser::NewStub(channel_);
                if (!channel_->WaitForConnected(std::chrono::system_clock::now()
                        + std::chrono::seconds(5))) {
                    throw std::runtime_error("Connection timeout");
                }
                connected_ = true;
                initiateAsyncCall();
                processAsyncResponses(stoken);
            } catch (const std::exception& e) {
                Logger::getLogger()->error(
                    "Worker error for {}: {}", sourceId_, e.what());
                Q_EMIT error(QString::fromStdString(e.what()));
            }
        });
    }

    void stop()
    {
        if (cq_)
            cq_->Shutdown();
        worker_.request_stop();
        worker_.join();
    }

    grpc::ChannelArguments createChannelArguments() const
    {
        grpc::ChannelArguments args;
        args.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, 64 * 1024 * 1024);
        args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 30000);
        args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 10000);
        args.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
        args.SetInt(GRPC_ARG_ENABLE_RETRIES, 1);
        return args;
    }

    void initiateAsyncCall()
    {
        auto call = std::make_unique<AsyncCall>();
        call->context.set_deadline(
            std::chrono::system_clock::now() + std::chrono::hours(24));
        call->reader = stub_->PrepareAsyncSubscribe(&call->context, cq_.get());
        call->reader->StartCall(call.get());
        call->state = AsyncCall::State::START;
        activeCalls_.push_back(std::move(call));
    }

    void processAsyncResponses(std::stop_token stoken)
    {
        void* tag;
        bool ok = false;
        while (!stoken.stop_requested() && cq_->Next(&tag, &ok)) {
            auto* call = static_cast<AsyncCall*>(tag);
            if (!ok) {
                connected_ = false;
                initiateAsyncCall();
                continue;
            }
            switch (call->state) {
            case AsyncCall::State::START:
                call->state = AsyncCall::State::WRITE;
                {
                    geyser::SubscribeRequest req;
                    req.set_commitment(geyser::CommitmentLevel::PROCESSED);
                    auto* txs_map = req.mutable_transactions();
                    geyser::SubscribeRequestFilterTransactions tx_filters;
                    tx_filters.set_vote(false);
                    tx_filters.set_failed(false);
                    (*txs_map)["tokens"] = tx_filters;
                    call->reader->Write(req, call);
                }
                break;
            case AsyncCall::State::WRITE:
                call->state = AsyncCall::State::READ;
                call->reader->Read(&call->response, call);
                break;
            case AsyncCall::State::READ:
                if (call->response.update_oneof_case()
                    == geyser::SubscribeUpdate::kTransaction) {
                    auto sig = call->response.transaction()
                                   .transaction()
                                   .signature();
                    batch_.emplace_back(
                        sig, call->response.SerializeAsString());
                    ++totalTransactions_;
                    if (batch_.size() >= 100) {
                        processBatch();
                        batch_.clear();
                        ++processedBatches_;
                    }
                }
                call->reader->Read(&call->response, call);
                break;
            case AsyncCall::State::FINISHED:
                call->reader->Finish(&call->status, call);
                connected_ = false;
                initiateAsyncCall();
                break;
            }
        }
    }

    void processBatch()
    {
        if (batch_.empty())
            return;

        try {
            std::vector<geyser::SubscribeUpdateTransaction> transactions;
            transactions.reserve(batch_.size());
            for (const auto& [sig, data] : batch_) {
                geyser::SubscribeUpdate update;
                if (!update.ParseFromString(data)) {
                    Logger::getLogger()->warn(
                        "Failed to parse transaction data for {}", sig);
                    continue;
                }
                if (update.update_oneof_case()
                    == geyser::SubscribeUpdate::kTransaction) {
                    transactions.push_back(update.transaction());
                }
            }

            size_t hits = filter_.processBatch(sourceId_, transactions);
            storage_.storeBatch(batch_);
            notification_.sendBatchNotifications(
                QString("Processed %1 transactions").arg(batch_.size()));
            nlohmann::json data;
            data["source_id"] = sourceId_;
            data["transactions"] = batch_.size();
            data["hits"] = hits;
            Q_EMIT dataReceived(sourceId_, data);
            Logger::getLogger()->debug(
                "Processed batch of {} transactions with {} filter hits",
                batch_.size(), hits);
        } catch (const std::exception& e) {
            Logger::getLogger()->error("Error processing batch: {}", e.what());
            Q_EMIT error(QString::fromStdString(e.what()));
        }
    }

    std::string sourceId_;
    std::string address_;
    StorageManager& storage_;
    NotificationManager& notification_;
    FilterManager& filter_;
    std::jthread worker_;
    std::unique_ptr<grpc::CompletionQueue> cq_;
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<geyser::Geyser::Stub> stub_;
    std::vector<std::unique_ptr<AsyncCall>> activeCalls_;
    std::vector<std::pair<std::string, std::string>> batch_;
    bool connected_ { false };
    uint64_t totalTransactions_ { 0 };
    uint64_t processedBatches_ { 0 };
};

class DataSourceManager : public QObject {
    Q_OBJECT

public:
    explicit DataSourceManager(ConfigManager& config, StorageManager& storage,
        NotificationManager& notifier, FilterManager& filter,
        QObject* parent = nullptr);
    ~DataSourceManager();

    std::string addDataSource(
        const std::string& address, const std::string& type);
    void removeDataSource(const std::string& sourceId);
    nlohmann::json getStats() const;
    void setHealthCheckInterval(std::chrono::seconds interval);

Q_SIGNALS:
    void dataReceived(const std::string& sourceId, const nlohmann::json& data);
    void sourceAdded(const std::string& sourceId);
    void sourceRemoved(const std::string& sourceId);
    void statsUpdated(const nlohmann::json& stats);

private:
    void performHealthCheck();
    ConfigManager& config_;
    StorageManager& storage_;
    NotificationManager& notification_;
    FilterManager& filter_;
    std::map<std::string, std::unique_ptr<GeyserClientWorker>> workers_;
    mutable std::mutex mutex_;
    QTimer healthCheckTimer_;
};
}
