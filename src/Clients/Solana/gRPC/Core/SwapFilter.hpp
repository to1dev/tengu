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

class SwapFilter : public TransactionFilter {
public:
    explicit SwapFilter(std::unordered_set<std::string> smartWallets,
        double minSwapAmount = 1.0);
    void processTransaction(const std::string& sourceId,
        const geyser::SubscribeUpdateTransaction& tx) override;
    void updateConfig(const std::string& config) override;

    std::string name() const override
    {
        return "SwapFilter";
    }

    json getRecentSwaps(size_t maxEntries = 100) const;

private:
    struct TokenBalance {
        std::string owner;
        std::string mint;
        double amount;
        std::string tokenName;
    };

    struct SwapInfo {
        std::string wallet;
        std::string tokenMint;
        std::string tokenName;
        double tokenAmount;
        double solAmount;
        std::string direction;
        std::string signature;
        std::chrono::system_clock::time_point timestamp;
    };

    const std::string WSOL_MINT = "So11111111111111111111111111111111111111112";
    std::unordered_set<std::string> smartWallets_;
    std::unordered_set<std::string> trackedTokens_;
    std::unordered_set<std::string> ignoredTokens_;
    std::unordered_map<std::string, std::string> tokenNames_;
    double minSwapAmount_;
    std::vector<SwapInfo> recentSwaps_;
    mutable std::mutex recentSwapsMutex_;
    const size_t maxRecentSwaps_ = 1000;
};
}
