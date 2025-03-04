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

namespace Daitengu::Clients::Solana {

SwapFilter::SwapFilter(const std::unordered_set<std::string>& smartWallets)
    : smartWallets_(smartWallets)
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

bool SwapFilter::isSmartWallet(const std::string& address) const
{
    return (smartWallets_.find(address) != smartWallets_.end());
}

void SwapFilter::reportSwap(const std::string& wallet,
    const std::string& tokenMint, double tokenChange,
    const std::unordered_map<std::string, double>& wsolChanges)
{
    double wsolChange = 0.0;
    for (const auto& [owner, change] : wsolChanges) {
        if (((tokenChange < 0) && (change > 0))
            || ((tokenChange > 0) && (change < 0))) {
            wsolChange = change;
            break;
        }
    }

    if (tokenChange < 0) {
        std::cout << "[SwapFilter] Wallet " << wallet << " sold "
                  << std::fabs(tokenChange) << " of token " << tokenMint
                  << " for " << wsolChange << " WSOL\n";
    } else {
        std::cout << "[SwapFilter] Wallet " << wallet << " bought "
                  << tokenChange << " of token " << tokenMint << " using "
                  << std::fabs(wsolChange) << " WSOL\n";
    }
    std::cout << "-------------------" << std::endl;
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

}
