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

#include "SwapFilter.h"

namespace Daitengu::Clients::Solana::gRPC {

SwapFilter::SwapFilter(std::unordered_set<std::string> smartWallets)
    : smartWallets_(std::move(smartWallets))
{
}

void SwapFilter::processTransaction(
    const geyser::SubscribeUpdateTransaction& tx)
{
    auto preBalances = getPreBalances(tx);
    auto postBalances = getPostBalances(tx);

    std::unordered_map<std::string, double> wsolChanges;
    for (const auto& pre : preBalances) {
        if (pre.mint == WSOL_MINT) {
            for (const auto& post : postBalances) {
                if (post.owner == pre.owner && post.mint == WSOL_MINT) {
                    double change = post.amount - pre.amount;
                    if (std::fabs(change) > 1e-6) {
                        wsolChanges[pre.owner] = change;
                    }
                }
            }
        }
    }

    for (const auto& pre : preBalances) {
        if (isSmartWallet(pre.owner) && pre.mint != WSOL_MINT) {
            for (const auto& post : postBalances) {
                if (post.owner == pre.owner && post.mint == pre.mint) {
                    double tokenChange = post.amount - pre.amount;
                    if (std::fabs(tokenChange) > 1e-6) {
                        reportSwap(
                            pre.owner, pre.mint, tokenChange, wsolChanges);
                    }
                }
            }
        }
    }
}

void SwapFilter::updateConfig(const std::string& config)
{
    try {
        auto json = json::parse(config);
        smartWallets_.clear();
        for (const auto& wallet : json["smart_wallets"]) {
            smartWallets_.insert(wallet.get<std::string>());
        }
        spdlog::info(
            "[SwapFilter] Updated smart wallets: {}", smartWallets_.size());
    } catch (const std::exception& e) {
        spdlog::error("[SwapFilter] Failed to update config: {}", e.what());
    }
}

bool SwapFilter::isSmartWallet(const std::string& address) const
{
    return smartWallets_.contains(address);
}

void SwapFilter::reportSwap(const std::string& wallet,
    const std::string& tokenMint, double tokenChange,
    const std::unordered_map<std::string, double>& wsolChanges)
{
    double wsolChange
        = wsolChanges.contains(wallet) ? wsolChanges.at(wallet) : 0.0;
    if (tokenChange < 0 && wsolChange > 0) {
        spdlog::info(
            "[SwapFilter] Wallet {} sold {:.6f} of token {} for {:.6f} WSOL",
            wallet, std::fabs(tokenChange), tokenMint, wsolChange);
    } else if (tokenChange > 0 && wsolChange < 0) {
        spdlog::info("[SwapFilter] Wallet {} bought {:.6f} of token {} using "
                     "{:.6f} WSOL",
            wallet, tokenChange, tokenMint, std::fabs(wsolChange));
    }
}

std::vector<SwapFilter::TokenBalance> SwapFilter::getPreBalances(
    const geyser::SubscribeUpdateTransaction& tx)
{
    std::vector<TokenBalance> balances;
    const auto& meta = tx.transaction().meta();
    for (const auto& tb : meta.pre_token_balances()) {
        balances.push_back(
            { tb.owner(), tb.mint(), tb.ui_token_amount().ui_amount() });
    }
    return balances;
}

std::vector<SwapFilter::TokenBalance> SwapFilter::getPostBalances(
    const geyser::SubscribeUpdateTransaction& tx)
{
    std::vector<TokenBalance> balances;
    const auto& meta = tx.transaction().meta();
    for (const auto& tb : meta.post_token_balances()) {
        balances.push_back(
            { tb.owner(), tb.mint(), tb.ui_token_amount().ui_amount() });
    }
    return balances;
}
}
