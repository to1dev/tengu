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

#include <array>
#include <cstdint>
#include <cstring>
#include <map>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include <sodium.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "izanagi/sha2.h"
#include "izanagi/sha3.h"

#include "izanagi/secp256k1/secp256k1.h"

#ifdef __cplusplus
}
#endif

#include "ChainWallet.h"

#include "../Utils/Hex.hpp"

namespace Daitengu::Wallets {

class EthereumWallet : public ChainWallet {
public:
    static inline constexpr std::string_view DEFAULT_DERIVATION_PATH
        = "m/44'/60'/0'/0";

    explicit EthereumWallet(
        bool useEip55 = false, Network::Type network = Network::Type::MAINNET);

    void fromPrivateKey(const std::string& privateKey) override;
    [[nodiscard]] std::string getAddress(std::uint32_t index = 0) override;
    [[nodiscard]] std::string getPrivateKey(std::uint32_t index = 0) override;
    [[nodiscard]] KeyPair deriveKeyPair(std::uint32_t index = 0) override;

protected:
    void onNetworkChanged() override;

private:
    void initNode(std::uint32_t index = 0) override;

    [[nodiscard]] std::string generateEthereumAddress() const;
    void fillUncompressedPublicKey();
    [[nodiscard]] std::string toEip55(const std::string& addressLower) const;

    unsigned char uncompressedPub_[65] { 0 };
    bool useEip55_ { false };
};

}
