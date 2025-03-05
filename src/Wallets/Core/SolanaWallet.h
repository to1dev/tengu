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

#ifndef SOLANAWALLET_H
#define SOLANAWALLET_H

#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include "Utils/Base58.hpp"

#include "ChainWallet.h"

namespace Daitengu::Wallets {

class SolanaWallet : public ChainWallet {
public:
    static inline constexpr std::string_view DEFAULT_DERIVATION_PATH
        = "m/44'/501'/0'";

    SolanaWallet(Network::Type network = Network::Type::MAINNET);

    bool solanaMode() const;
    void setSolanaMode(bool newSolanaMode);

    void fromPrivateKey(const std::string& privateKey) override;
    [[nodiscard]] std::string getAddress(std::uint32_t index = 0) override;
    [[nodiscard]] std::string getPrivateKey(std::uint32_t index = 0) override;
    [[nodiscard]] KeyPair deriveKeyPair(std::uint32_t index = 0) override;

protected:
    const std::map<Network::Type, ChainNetwork> networkConfigs_ = {
        {
            Network::Type::MAINNET,
            {
                Network::Type::MAINNET,
                "mainnet-beta",
                101,
                AddressEncoding::BASE58,
                "",
                {},
                "",
                0,
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
                102,
                AddressEncoding::BASE58,
                "",
                {},
                "",
                0,
                false,
                nullptr,
                nullptr,
            },
        },
        {
            Network::Type::DEVNET,
            {
                Network::Type::DEVNET,
                "devnet",
                103,
                AddressEncoding::BASE58,
                "",
                {},
                "",
                0,
                false,
                nullptr,
                nullptr,
            },
        },
    };

    void onNetworkChanged() override;

private:
    void initNode(std::uint32_t index = 0) override;

private:
    bool solanaMode_ { false };
};

}
#endif // SOLANAWALLET_H
