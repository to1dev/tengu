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

#include "FilterManager.hpp"

#include "../Utils/Logger.hpp"

namespace solana {

FilterManager::FilterManager(QObject* parent)
    : QObject(parent)
    , maxThreads_(std::thread::hardware_concurrency() / 2)
{
}

FilterManager::~FilterManager() = default;

void FilterManager::addFilter(
    const std::string& name, std::shared_ptr<TransactionFilter> filter)
{
    std::lock_guard lock(mutex_);
    filters_[name] = std::move(filter);
    filterHits_[name] = 0;
    Logger::getLogger()->info("Filter added: {}", name);
    Q_EMIT filterAdded(QString::fromStdString(name));
}

void FilterManager::removeFilter(const std::string& name)
{
    std::lock_guard lock(mutex_);
    filters_.erase(name);
    filterHits_.erase(name);
    Logger::getLogger()->info("Filter removed: {}", name);
    Q_EMIT filterRemoved(QString::fromStdString(name));
}

void FilterManager::updateFilterConfig(
    const std::string& name, const std::string& config)
{
    std::lock_guard lock(mutex_);
    if (auto it = filters_.find(name); it != filters_.end()) {
        try {
            it->second->updateConfig(config);
            Logger::getLogger()->info("Filter config updated: {}", name);
            Q_EMIT filterUpdated(QString::fromStdString(name));
        } catch (const std::exception& e) {
            Logger::getLogger()->error(
                "Error updating filter {}: {}", name, e.what());
        }
    }
}

std::shared_ptr<TransactionFilter> FilterManager::getFilter(
    const std::string& name) const
{
    std::lock_guard lock(mutex_);
    return filters_.count(name) ? filters_.at(name) : nullptr;
}

std::vector<std::string> FilterManager::getFilterNames() const
{
    std::lock_guard lock(mutex_);
    std::vector<std::string> names;
    names.reserve(filters_.size());
    for (const auto& [name, _] : filters_) {
        names.push_back(name);
    }
    return names;
}

size_t FilterManager::processTransaction(
    const std::string& sourceId, const geyser::SubscribeUpdateTransaction& tx)
{
    std::lock_guard lock(mutex_);
    std::vector<std::future<bool>> futures;
    futures.reserve(filters_.size());
    for (const auto& [name, filter] : filters_) {
        futures.push_back(std::async(
            std::launch::async, [this, &sourceId, &tx, filter, name]() {
                try {
                    filter->processTransaction(sourceId, tx);
                    std::lock_guard lock(mutex_);
                    filterHits_[name]++;
                    return true;
                } catch (const std::exception& e) {
                    Logger::getLogger()->error(
                        "Error in filter {}: {}", name, e.what());
                    return false;
                }
            }));
    }
    return std::count_if(
        futures.begin(), futures.end(), [](auto& f) { return f.get(); });
}

size_t FilterManager::processBatch(const std::string& sourceId,
    const std::vector<geyser::SubscribeUpdateTransaction>& transactions)
{
    if (transactions.empty())
        return 0;

    std::vector<std::future<size_t>> futures;
    futures.reserve(transactions.size());
    for (const auto& tx : transactions) {
        futures.push_back(
            std::async(std::launch::async, [this, &sourceId, &tx]() {
                return processTransaction(sourceId, tx);
            }));
    }

    size_t totalHits = 0;
    for (auto& future : futures) {
        totalHits += future.get();
    }

    return totalHits;
}

void FilterManager::setMaxConcurrentFilters(int maxThreads)
{
    maxThreads_ = std::max(1,
        std::min(
            maxThreads, static_cast<int>(std::thread::hardware_concurrency())));
    Logger::getLogger()->info("Max concurrent filters set to {}", maxThreads_);
}

json FilterManager::getFilterStats() const
{
    json stats;
    std::lock_guard lock(mutex_);
    for (const auto& [name, hits] : filterHits_) {
        stats[name] = hits;
    }
    return stats;
}
}
