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

#ifndef BITCOINWALLET_H
#define BITCOINWALLET_H

#include "ChainWallet.h"

namespace Daitengu::Wallets {

struct BitcoinNetwork : public Network {
    enum class Type {
        REGTEST = static_cast<int>(Network::Type::DEVNET) + 1,
        TEST4 = REGTEST + 1
    };
};

class BitcoinWallet : public ChainWallet {
public:
    BitcoinWallet(Network::Type network = Network::Type::MAINNET);

protected:
    const std::map<Network::Type, ChainNetwork> networkConfigs_ = {
        {
            Network::Type::MAINNET,
            {
                Network::Type::MAINNET,
                "mainnet",
                1,
                AddressEncoding::BECH32M,
                "",
                { 0x00 },
                "bc",
                0x00,
                false,
                nullptr,
                nullptr,
            },
        },
        {
            Network::Type::TESTNET,
            {
                Network::Type::TESTNET,
                "testnet",
                2,
                AddressEncoding::BECH32M,
                "",
                { 0x6F },
                "tb",
                0x6F,
                false,
                nullptr,
                nullptr,
            },
        },
        {
            static_cast<Network::Type>(BitcoinNetwork::Type::REGTEST),
            {
                static_cast<Network::Type>(BitcoinNetwork::Type::REGTEST),
                "regtest",
                2,
                AddressEncoding::BECH32M,
                "",
                { 0x6F },
                "tb",
                0x6F,
                false,
                nullptr,
                nullptr,
            },
        },
    };

    void onNetworkChanged() override;
};

}
#endif // BITCOINWALLET_H
