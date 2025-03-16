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

#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace Daitengu::Wallets {

enum class ChainType {
    UNKNOWN = -1,
    BITCOIN,
    ETHEREUM,
    SOLANA,
    SUI,
};

enum class WalletType {
    Unknown = -1,
    Original = 0,
    Mnemonic,
    Priv,
    Wif,
    Address,
};

struct Network {
    enum class Type {
        MAINNET,
        TESTNET,
        DEVNET,
    };
};

enum class AddressEncoding {
    BASE58CHECK,
    BECH32,
    BECH32M,
    HEX,
    BASE58,
    SS58,
    CUSTOM,
};

struct ChainConfig {
    std::uint32_t coinType;
    std::string curveName;
    bool useHardenedChange;
    std::vector<std::uint32_t> basePath;
};

struct ChainNetwork {
    Network::Type type;
    std::string networkName;
    std::uint64_t chainId;
    AddressEncoding addressEncoding;
    std::string addressPrefix;
    std::vector<std::uint8_t> prefixBytes;
    std::string hrp;
    std::uint8_t versionByte;
    bool supportsEip55;
    std::function<std::string(const std::vector<std::uint8_t>&)> customEncoder;
    std::function<bool(const std::string&)> customValidator;
};

}
#endif // TYPES_H
