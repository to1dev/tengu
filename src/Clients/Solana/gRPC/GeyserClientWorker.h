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

#include <chrono>

using namespace std::chrono_literals;

#include <grpcpp/completion_queue.h>
#include <grpcpp/grpcpp.h>

#include <spdlog/spdlog.h>

#include <qcoro/QCoro>

#include "FilterManager.h"
#include "NotificationManager.h"
#include "StorageManager.h"

namespace Daitengu::Clients::Solana::gRPC {

class GeyserClientWorker : public QObject {
    Q_OBJECT

public:
    GeyserClientWorker(QString address,
        std::vector<std::shared_ptr<TransactionFilter>> filters,
        StorageManager* storage, NotificationManager* notifier,
        FilterManager* filterMgr, QObject* parent = nullptr);
    ~GeyserClientWorker();

    QCoro::Task<void> start();
    void stop();

Q_SIGNALS:
    void updateReceived(const QString& update);
    void transactionProcessed(size_t filterHits);

private:
    QCoro::Task<void> runConnection();
    QCoro::Task<void> processBatch(
        std::vector<std::pair<std::string, std::string>>& batch);
    QCoro::Task<bool> asyncRead(
        std::unique_ptr<grpc::ClientAsyncReaderWriter<geyser::SubscribeRequest,
            geyser::SubscribeUpdate>>& stream,
        geyser::SubscribeUpdate& update);

    QString address_;
    std::atomic_bool shouldRun_ { false };
    std::vector<std::shared_ptr<TransactionFilter>> initialFilters_;
    StorageManager* storageManager_;
    NotificationManager* notificationManager_;
    FilterManager* filterManager_;
    std::unique_ptr<grpc::CompletionQueue> cq_;
};
}
