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
#include <memory>
#include <thread>

#include <boost/asio/io_context.hpp>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include <spdlog/spdlog.h>

#include <qcoro/QCoro>

#include "ConfigManager.h"
#include "FilterManager.h"
#include "GeyserClientWorker.h"
#include "HttpServer.h"
#include "NotificationManager.h"
#include "StorageManager.h"

namespace Daitengu::Clients::Solana::gRPC {

class GeyserClientManager : public QObject {
    Q_OBJECT

public:
    explicit GeyserClientManager(
        const std::string& configFile, QObject* parent = nullptr);
    ~GeyserClientManager();

    QCoro::Task<GeyserClientWorker*> createWorker();
    QCoro::Task<void> removeWorker(GeyserClientWorker* worker);
    void stopAllWorkers();
    void startInThread(GeyserClientWorker* worker);

    FilterManager& getFilterManager()
    {
        return filterManager_;
    }

    NotificationManager& getNotificationManager()
    {
        return notificationManager_;
    }

    void startWebUI();
    void updateFilter(const std::string& name, const std::string& config);
    void logStats() const;
    json getStats() const;

Q_SIGNALS:
    void updateReceived(const QString& msg);

private:
    QCoro::Task<std::string> selectGeyserAddress();

    std::vector<std::unique_ptr<GeyserClientWorker>> workers_;
    ConfigManager configManager_;
    StorageManager storageManager_;
    NotificationManager notificationManager_;
    FilterManager filterManager_;
    std::unique_ptr<HttpServer> httpServer_;

    std::unique_ptr<boost::asio::io_context> io_context_;
    std::unique_ptr<std::thread> io_thread_;

    std::atomic<size_t> transactionCount_ { 0 };
    std::atomic<size_t> filterHits_ { 0 };
    std::chrono::steady_clock::time_point startTime_;
};

}
