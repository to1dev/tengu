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

#include "FilterManager.h"

namespace Daitengu::Clients::Solana::gRPC {

void FilterManager::addFilter(
    const std::string& name, std::shared_ptr<TransactionFilter> filter)
{
    std::lock_guard<std::mutex> lock(cacheMutex_);
    filters_[name] = std::move(filter);
    spdlog::info("Filter added: {}", name);
}

void FilterManager::removeFilter(const std::string& name)
{
    std::lock_guard<std::mutex> lock(cacheMutex_);
    filters_.erase(name);
    spdlog::info("Filter removed: {}", name);
}

void FilterManager::updateFilterConfig(
    const std::string& name, const std::string& config)
{
    std::lock_guard<std::mutex> lock(cacheMutex_);
    auto it = filters_.find(name);
    if (it != filters_.end()) {
        it->second->updateConfig(config);
        spdlog::info("Filter config updated: {}", name);
    } else {
        spdlog::warn("Filter not found: {}", name);
    }
}

std::shared_ptr<TransactionFilter> FilterManager::getFilter(
    const std::string& name) const
{
    std::lock_guard<std::mutex> lock(cacheMutex_);
    auto it = filters_.find(name);
    return it != filters_.end() ? it->second : nullptr;
}

QCoro::Task<size_t> FilterManager::processTransaction(
    const geyser::SubscribeUpdateTransaction& tx)
{
    size_t hits = 0;
    std::vector<QFuture<void>> tasks;
    {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        for (const auto& [name, filter] : filters_) {
            auto future = QtConcurrent::run(
                [filter, &tx]() { filter->processTransaction(tx); });
            tasks.push_back(future);
            hits++;
        }
    }
    for (auto& task : tasks)
        co_await task;

    co_return hits;
}
}
