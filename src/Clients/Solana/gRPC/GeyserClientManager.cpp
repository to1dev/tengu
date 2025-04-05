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

#include "GeyserClientManager.h"

namespace Daitengu::Clients::Solana::gRPC {

GeyserClientManager::GeyserClientManager(
    const std::string& configFile, QObject* parent)
    : QObject(parent)
    , configManager_(configFile)
    , storageManager_(configManager_.getDbPath())
    , notificationManager_()
    , startTime_(std::chrono::steady_clock::now())
{
}

GeyserClientManager::~GeyserClientManager()
{
    stopAllWorkers();

    if (io_context_) {
        io_context_->stop();
    }

    if (io_thread_ && io_thread_->joinable()) {
        io_thread_->join();
    }
}

QCoro::Task<std::string> GeyserClientManager::selectGeyserAddress()
{
    auto addresses = configManager_.getGeyserAddresses();
    if (addresses.empty())
        throw std::runtime_error("No Geyser addresses configured");

    static size_t index = 0;
    co_return addresses[index++ % addresses.size()];
}

QCoro::Task<GeyserClientWorker*> GeyserClientManager::createWorker()
{
    auto address = co_await selectGeyserAddress();
    auto worker
        = std::make_unique<GeyserClientWorker>(QString::fromStdString(address),
            std::vector<std::shared_ptr<TransactionFilter>> {},
            &storageManager_, &notificationManager_, &filterManager_, this);
    GeyserClientWorker* workerPtr = worker.get();

    connect(workerPtr, &GeyserClientWorker::updateReceived, this,
        &GeyserClientManager::updateReceived);
    connect(workerPtr, &GeyserClientWorker::transactionProcessed, this,
        [this](size_t hits) {
            transactionCount_ += hits > 0 ? 1 : 0;
            filterHits_ += hits;
        });

    co_await workerPtr->start();
    workers_.push_back(std::move(worker));
    spdlog::info("Worker created for address: {}", address);
    co_return workerPtr;
}

QCoro::Task<void> GeyserClientManager::removeWorker(GeyserClientWorker* worker)
{
    if (!worker)
        co_return;

    worker->stop();
    auto it = std::ranges::find_if(
        workers_, [worker](const auto& w) { return w.get() == worker; });
    if (it != workers_.end())
        workers_.erase(it);

    spdlog::info("Worker removed");
    co_return;
}

void GeyserClientManager::stopAllWorkers()
{
    for (auto& worker : workers_)
        worker->stop();
    workers_.clear();
    spdlog::info("All workers stopped");
}

void GeyserClientManager::startInThread(GeyserClientWorker* worker)
{
    spdlog::warn("Threaded start not implemented");
}

void GeyserClientManager::startWebUI()
{
    io_context_ = std::make_unique<boost::asio::io_context>();

    httpServer_ = std::make_unique<HttpServer>(*io_context_, 8080);
    httpServer_->start();

    io_thread_ = std::make_unique<std::thread>([this]() {
        spdlog::info("Web UI started on port 8080");
        io_context_->run();
    });
}

void GeyserClientManager::updateFilter(
    const std::string& name, const std::string& config)
{
    filterManager_.updateFilterConfig(name, config);
}

void GeyserClientManager::logStats() const
{
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - startTime_)
                       .count();
    double txRate
        = elapsed > 0 ? static_cast<double>(transactionCount_) / elapsed : 0;
    spdlog::info("Stats: Transactions: {}, Filter Hits: {}, Tx/sec: {:.2f}",
        transactionCount_.load(), filterHits_.load(), txRate);
}

json GeyserClientManager::getStats() const
{
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - startTime_)
                       .count();
    double txRate
        = elapsed > 0 ? static_cast<double>(transactionCount_) / elapsed : 0;
    json stats;
    stats["transaction_count"] = transactionCount_.load();
    stats["filter_hits"] = filterHits_.load();
    stats["tx_per_sec"] = txRate;

    return stats;
}
}
