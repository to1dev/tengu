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

#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include <QObject>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "TransactionFilter.hpp"

namespace solana {

class FilterManager : public QObject {
    Q_OBJECT

public:
    explicit FilterManager(QObject* parent = nullptr);
    ~FilterManager();

    void addFilter(
        const std::string& name, std::shared_ptr<TransactionFilter> filter);
    void removeFilter(const std::string& name);
    void updateFilterConfig(const std::string& name, const std::string& config);
    std::shared_ptr<TransactionFilter> getFilter(const std::string& name) const;
    std::vector<std::string> getFilterNames() const;
    size_t processTransaction(const std::string& sourceId,
        const geyser::SubscribeUpdateTransaction& tx);
    size_t processBatch(const std::string& sourceId,
        const std::vector<geyser::SubscribeUpdateTransaction>& transactions);
    void setMaxConcurrentFilters(int maxThreads);
    json getFilterStats() const;

Q_SIGNALS:
    void filterAdded(const QString& name);
    void filterRemoved(const QString& name);
    void filterUpdated(const QString& name);

private:
    std::map<std::string, std::shared_ptr<TransactionFilter>> filters_;
    std::map<std::string, uint64_t> filterHits_;
    mutable std::mutex mutex_;
    int maxThreads_;
};
}
