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

#include "ChainWallet.h"

namespace Daitengu::Wallets {

ChainWallet::ChainWallet(ChainType chainType, Network::Type network)
    : chainType_(chainType)
    , currentNetwork_(network)
{
    initChainConfig();
}

void ChainWallet::switchNetwork(Network::Type network)
{
    currentNetwork_ = network;
}

Network::Type ChainWallet::currentNetwork() const
{
    return currentNetwork_;
}

const ChainNetwork& ChainWallet::getCurrentNetworkConfig() const
{
    auto it = networkConfigs_.find(currentNetwork_);
    if (it == networkConfigs_.end()) {
        throw std::runtime_error("Unsupported network type.");
    }
    return it->second;
}

void ChainWallet::initChainConfig()
{
    static const std::map<ChainType, ChainConfig> configs = {
        { ChainType::SOLANA,
            { 501, "ed25519", true,
                { 44 | HARDENED, 501 | HARDENED, 0 | HARDENED,
                    0 | HARDENED } } },
        { ChainType::ETHEREUM,
            {
                60, "secp256k1", false,
                { 44 | HARDENED, 60 | HARDENED, 0 | HARDENED, 0 } // base path
            } },
        { ChainType::BITCOIN,
            { 0, "secp256k1", false,
                { 84 | HARDENED, 0 | HARDENED, 0 | HARDENED, 0 }

            } }
    };

    auto it = configs.find(chainType_);
    if (it == configs.end()) {
        throw std::runtime_error("Unsupported chain type.");
    }

    config_ = it->second;
}

}
