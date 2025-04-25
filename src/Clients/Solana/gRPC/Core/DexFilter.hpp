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
#include <unordered_set>
#include <vector>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "TransactionFilter.hpp"

namespace solana {

class DexFilter : public TransactionFilter {
public:
    explicit DexFilter(std::unordered_set<std::string> dexPrograms);
    void processTransaction(const std::string& sourceId,
        const geyser::SubscribeUpdateTransaction& tx) override;
    void updateConfig(const std::string& config) override;

    std::string name() const override
    {
        return "DexFilter";
    }

    json getRecentDexTransactions(size_t maxEntries = 100) const;

private:
    struct DexTransactionInfo {
        std::string signature;
        std::string dexProgram;
        std::string dexName;
        std::optional<std::string> market;
        std::optional<std::string> marketName;
        std::vector<std::string> accounts;
        std::chrono::system_clock::time_point timestamp;
    };

    std::unordered_set<std::string> dexPrograms_;
    std::unordered_map<std::string, std::string> dexNames_;
    std::unordered_set<std::string> marketWhitelist_;
    std::unordered_set<std::string> marketBlacklist_;
    std::unordered_map<std::string, std::string> marketNames_;
    std::vector<DexTransactionInfo> recentTransactions_;
    mutable std::mutex recentTxMutex_;
    const size_t maxRecentTransactions_ = 1000;
};
}
