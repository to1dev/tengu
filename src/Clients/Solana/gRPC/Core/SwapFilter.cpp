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

#include "SwapFilter.hpp"

#include "../Utils/Logger.hpp"

namespace solana {

SwapFilter::SwapFilter(
    std::unordered_set<std::string> smartWallets, double minSwapAmount)
    : TransactionFilter()
    , smartWallets_(std::move(smartWallets))
    , minSwapAmount_(minSwapAmount)
{
    tokenNames_[WSOL_MINT] = "SOL";
    tokenNames_["EPjFWdd5AufqSSqeM2qN1xzybapC8G4wEGGkZwyTDt1v"] = "USDC";
    Logger::getLogger()->info(
        "SwapFilter initialized with {} wallets, min swap: {}",
        smartWallets_.size(), minSwapAmount_);
}

void SwapFilter::processTransaction(
    const std::string& sourceId, const geyser::SubscribeUpdateTransaction& tx)
{
    incrementProcessCount();
    try {
        const auto& meta = tx.transaction().meta();
        std::vector<TokenBalance> preBalances, postBalances;
        for (const auto& tb : meta.pre_token_balances()) {
            preBalances.push_back(
                { tb.owner(), tb.mint(), tb.ui_token_amount().ui_amount(),
                    tokenNames_.count(tb.mint()) ? tokenNames_.at(tb.mint())
                                                 : tb.mint().substr(0, 8) });
        }
        for (const auto& tb : meta.post_token_balances()) {
            postBalances.push_back(
                { tb.owner(), tb.mint(), tb.ui_token_amount().ui_amount(),
                    tokenNames_.count(tb.mint()) ? tokenNames_.at(tb.mint())
                                                 : tb.mint().substr(0, 8) });
        }

        std::unordered_map<std::string, double> wsolChanges;
        for (const auto& pre : preBalances) {
            if (pre.mint == WSOL_MINT) {
                for (const auto& post : postBalances) {
                    if (post.owner == pre.owner && post.mint == WSOL_MINT) {
                        double change = post.amount - pre.amount;
                        if (std::fabs(change) > 1e-6)
                            wsolChanges[pre.owner] = change;
                    }
                }
            }
        }

        bool matched = false;
        for (const auto& pre : preBalances) {
            if (smartWallets_.contains(pre.owner) && pre.mint != WSOL_MINT) {
                for (const auto& post : postBalances) {
                    if (post.owner == pre.owner && post.mint == pre.mint) {
                        double tokenChange = post.amount - pre.amount;
                        if (std::fabs(tokenChange) > 1e-6
                            && std::fabs(tokenChange) >= minSwapAmount_) {
                            SwapInfo swap { pre.owner, pre.mint, pre.tokenName,
                                std::fabs(tokenChange),
                                wsolChanges.count(pre.owner)
                                    ? std::fabs(wsolChanges.at(pre.owner))
                                    : 0.0,
                                tokenChange > 0 ? "buy" : "sell",
                                tx.transaction().signature(),
                                std::chrono::system_clock::now() };
                            {
                                std::lock_guard lock(recentSwapsMutex_);
                                recentSwaps_.push_back(std::move(swap));
                                if (recentSwaps_.size() > maxRecentSwaps_) {
                                    recentSwaps_.erase(recentSwaps_.begin());
                                }
                            }
                            incrementMatchCount();
                            Logger::getLogger()->info(
                                "Swap detected: {} {} {:.6f} {} (tx: {})",
                                pre.owner, swap.direction, tokenChange,
                                pre.tokenName, tx.transaction().signature());

                            // Emit signal to notify GUI
                            json swapInfo;
                            swapInfo["wallet"] = swap.wallet;
                            swapInfo["mint"] = swap.tokenMint;
                            swapInfo["token_name"] = swap.tokenName;
                            swapInfo["token_amount"] = swap.tokenAmount;
                            swapInfo["sol_amount"] = swap.solAmount;
                            swapInfo["direction"] = swap.direction;
                            swapInfo["signature"] = swap.signature;
                            swapInfo["timestamp"] = std::chrono::duration_cast<
                                std::chrono::milliseconds>(
                                swap.timestamp.time_since_epoch())
                                                        .count();
                            matched = true;
                        }
                    }
                }
            }
        }

        return;
    } catch (const std::exception& e) {
        Logger::getLogger()->error(
            "SwapFilter error processing transaction: {}", e.what());
        return;
    }
}

void SwapFilter::updateConfig(const std::string& config)
{
    try {
        auto json = json::parse(config);
        std::lock_guard lock(mutex_);
        if (json.contains("smart_wallets")) {
            smartWallets_.clear();
            for (const auto& wallet : json["smart_wallets"]) {
                smartWallets_.insert(wallet.get<std::string>());
            }
            Logger::getLogger()->info(
                "Updated smart wallets: {}", smartWallets_.size());
        }
        if (json.contains("min_swap_amount")) {
            minSwapAmount_ = json["min_swap_amount"].get<double>();
            Logger::getLogger()->info(
                "Updated min swap amount: {}", minSwapAmount_);
        }
        if (json.contains("tracked_tokens")) {
            trackedTokens_.clear();
            for (const auto& token : json["tracked_tokens"]) {
                trackedTokens_.insert(token.get<std::string>());
            }
            Logger::getLogger()->info(
                "Updated tracked tokens: {}", trackedTokens_.size());
        }
        if (json.contains("ignored_tokens")) {
            ignoredTokens_.clear();
            for (const auto& token : json["ignored_tokens"]) {
                ignoredTokens_.insert(token.get<std::string>());
            }
            Logger::getLogger()->info(
                "Updated ignored tokens: {}", ignoredTokens_.size());
        }
        if (json.contains("token_names")) {
            for (const auto& [mint, name] : json["token_names"].items()) {
                tokenNames_[mint] = name.get<std::string>();
            }
            Logger::getLogger()->info(
                "Updated token names: {}", tokenNames_.size());
        }
    } catch (const std::exception& e) {
        Logger::getLogger()->error(
            "Failed to update SwapFilter config: {}", e.what());
    }
}

json SwapFilter::getRecentSwaps(size_t maxEntries) const
{
    std::lock_guard lock(recentSwapsMutex_);
    json result = json::array();
    size_t count = 0;
    for (auto it = recentSwaps_.rbegin();
        it != recentSwaps_.rend() && count < maxEntries; ++it, ++count) {
        json swap;
        swap["wallet"] = it->wallet;
        swap["token_mint"] = it->tokenMint;
        swap["token_name"] = it->tokenName;
        swap["token_amount"] = it->tokenAmount;
        swap["sol_amount"] = it->solAmount;
        swap["direction"] = it->direction;
        swap["signature"] = it->signature;
        auto time = std::chrono::system_clock::to_time_t(it->timestamp);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time), "%Y-%m-%d %H:%M:%S");
        swap["timestamp"] = ss.str();
        result.push_back(swap);
    }
    return result;
}
}
