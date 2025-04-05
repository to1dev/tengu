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

#include <mutex>
#include <unordered_map>

#include <QtConcurrent>

#include <spdlog/spdlog.h>

#include <qcoro/QCoro>

#include "TransactionFilter.h"

namespace Daitengu::Clients::Solana::gRPC {

class FilterManager {
public:
    FilterManager() = default;

    void addFilter(
        const std::string& name, std::shared_ptr<TransactionFilter> filter);
    void removeFilter(const std::string& name);
    void updateFilterConfig(const std::string& name, const std::string& config);
    std::shared_ptr<TransactionFilter> getFilter(const std::string& name) const;

    QCoro::Task<size_t> processTransaction(
        const geyser::SubscribeUpdateTransaction& tx);

private:
    std::unordered_map<std::string, std::shared_ptr<TransactionFilter>>
        filters_;
    mutable std::mutex cacheMutex_;
};

}
