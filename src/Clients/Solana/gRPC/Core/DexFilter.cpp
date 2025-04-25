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

#include "DexFilter.hpp"

#include "../Utils/Logger.hpp"

namespace solana {

DexFilter::DexFilter(std::unordered_set<std::string> dexPrograms)
    : dexPrograms_(std::move(dexPrograms))
{
    dexNames_["675kPX9MHTjS2zt1qfr1NYHuzeLXfQM9H24wFSUt1Mp8"] = "Raydium";
    dexNames_["6EF8rrecthR5Dkzon8Nwu78hRvfCKubJ14M5uBEwF6P"] = "Pump.fun";
    Logger::getLogger()->info(
        "DexFilter initialized with {} programs", dexPrograms_.size());
}

void DexFilter::processTransaction(
    const std::string& sourceId, const geyser::SubscribeUpdateTransaction& tx)
{
    incrementProcessCount();
    try {
        const auto& transaction = tx.transaction();
        const auto& signature = transaction.signature();
        const auto& accountKeys
            = transaction.transaction().message().account_keys();
        const auto& instructions
            = transaction.transaction().message().instructions();

        for (const auto& instr : instructions) {
            uint32_t programIdIndex = instr.program_id_index();
            if (programIdIndex >= accountKeys.size())
                continue;
            const std::string& programId = accountKeys[programIdIndex];
            if (!dexPrograms_.contains(programId))
                continue;

            std::vector<std::string> accounts;
            for (uint32_t accountIdx : instr.accounts()) {
                if (accountIdx < accountKeys.size())
                    accounts.push_back(accountKeys[accountIdx]);
            }
            std::string dexName = dexNames_.count(programId)
                ? dexNames_.at(programId)
                : "Unknown DEX";
            DexTransactionInfo info { signature, programId, dexName, {}, {},
                accounts, std::chrono::system_clock::now() };
            {
                std::lock_guard lock(recentTxMutex_);
                recentTransactions_.push_back(std::move(info));
                if (recentTransactions_.size() > maxRecentTransactions_) {
                    recentTransactions_.erase(recentTransactions_.begin());
                }
            }
            incrementMatchCount();
            Logger::getLogger()->info(
                "DEX transaction detected: {} (Program: {}, DEX: {})",
                signature, programId, dexName);
            break;
        }
    } catch (const std::exception& e) {
        Logger::getLogger()->error("DexFilter error: {}", e.what());
    }
}

void DexFilter::updateConfig(const std::string& config)
{
    try {
        auto json = json::parse(config);
        std::lock_guard lock(mutex_);
        if (json.contains("dex_programs")) {
            dexPrograms_.clear();
            for (const auto& program : json["dex_programs"]) {
                dexPrograms_.insert(program.get<std::string>());
            }
            Logger::getLogger()->info(
                "Updated DEX programs: {}", dexPrograms_.size());
        }
    } catch (const std::exception& e) {
        Logger::getLogger()->error(
            "Failed to update DexFilter config: {}", e.what());
    }
}

json DexFilter::getRecentDexTransactions(size_t maxEntries) const
{
    std::lock_guard lock(recentTxMutex_);
    json result = json::array();
    size_t count = 0;
    for (auto it = recentTransactions_.rbegin();
        it != recentTransactions_.rend() && count < maxEntries; ++it, ++count) {
        json tx;
        tx["signature"] = it->signature;
        tx["dex_program"] = it->dexProgram;
        tx["dex_name"] = it->dexName;
        result.push_back(tx);
    }

    return result;
}
}
