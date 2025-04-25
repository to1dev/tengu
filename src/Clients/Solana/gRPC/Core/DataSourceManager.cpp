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

#include "DataSourceManager.hpp"

#include <chrono>
#include <random>

#include <boost/uuid.hpp>

namespace solana {

DataSourceManager::DataSourceManager(ConfigManager& config,
    StorageManager& storage, NotificationManager& notifier,
    FilterManager& filter, QObject* parent)
    : QObject(parent)
    , config_(config)
    , storage_(storage)
    , notification_(notifier)
    , filter_(filter)
{
    connect(&healthCheckTimer_, &QTimer::timeout, this,
        &DataSourceManager::performHealthCheck);
    healthCheckTimer_.start(config_.getHealthCheckIntervalSeconds() * 1000);
    for (const auto& src : config_.getDataSources()) {
        addDataSource(src.address, src.type);
    }
}

DataSourceManager::~DataSourceManager()
{
    std::lock_guard lock(mutex_);
    workers_.clear();
}

std::string DataSourceManager::addDataSource(
    const std::string& address, const std::string& type)
{
    boost::uuids::random_generator generator;
    boost::uuids::uuid id = generator();

    std::string sourceId = boost::uuids::to_string(id);
    std::lock_guard lock(mutex_);
    workers_[sourceId] = std::make_unique<GeyserClientWorker>(
        sourceId, address, storage_, notification_, filter_);
    Logger::getLogger()->info("Added data source: {} ({})", sourceId, address);
    Q_EMIT sourceAdded(sourceId);
    return sourceId;
}

void DataSourceManager::removeDataSource(const std::string& sourceId)
{
    std::lock_guard lock(mutex_);
    workers_.erase(sourceId);
    Logger::getLogger()->info("Removed data source: {}", sourceId);
    Q_EMIT sourceRemoved(sourceId);
}

nlohmann::json DataSourceManager::getStats() const
{
    json stats;
    std::lock_guard lock(mutex_);
    for (const auto& [id, worker] : workers_) {
        json workerStats;
        workerStats["address"] = worker->getAddress();
        workerStats["connected"] = worker->isConnected();
        workerStats["transactions"] = worker->getTotalTransactions();
        workerStats["batches"] = worker->getProcessedBatches();
        stats["workers"][id] = workerStats;
    }
    return stats;
}

void DataSourceManager::setHealthCheckInterval(std::chrono::seconds interval)
{
    healthCheckTimer_.setInterval(interval.count() * 1000);
    Logger::getLogger()->info(
        "Health check interval set to {} seconds", interval.count());
}

void DataSourceManager::performHealthCheck()
{
    std::lock_guard lock(mutex_);
    for (auto it = workers_.begin(); it != workers_.end();) {
        if (!it->second->isConnected()) {
            Logger::getLogger()->warn("Unhealthy worker: {}", it->first);
            it = workers_.erase(it);
            addDataSource(it->second->getAddress(), "geyser");
        } else {
            ++it;
        }
    }
}
}
