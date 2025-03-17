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

#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include <sodium.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "izanagi/ripemd160.h"
#include "izanagi/segwit_addr.h"
#include "izanagi/sha2.h"

#include "izanagi/secp256k1/secp256k1.h"
#include "izanagi/secp256k1/secp256k1_extrakeys.h"

#ifdef __cplusplus
}
#endif

#include "Utils/Base58.hpp"

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
    static inline constexpr std::string_view DEFAULT_DERIVATION_PATH
        = "m/86'/0'/0'";

    explicit BitcoinWallet(
        bool useTaproot = true, Network::Type network = Network::Type::MAINNET);

    static bool isValid(std::string_view address);

    void fromPrivateKey(const std::string& privateKey) override;
    [[nodiscard]] std::string getAddress(std::uint32_t index = 0) override;
    [[nodiscard]] std::string getPrivateKey(std::uint32_t index = 0) override;
    [[nodiscard]] KeyPair deriveKeyPair(std::uint32_t index = 0) override;

    [[nodiscard]] std::string getScriptPubKey(std::uint32_t index = 0);
    [[nodiscard]] std::string getScriptHash(std::uint32_t index = 0);

protected:
    const std::map<Network::Type, ChainNetwork> networkConfigs_ = {
        {
            Network::Type::MAINNET,
            {
                Network::Type::MAINNET,
                "mainnet",
                0,
                AddressEncoding::BECH32M,
                "",
                { 0x00 },
                "bc",
                0x80,
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
                1,
                AddressEncoding::BECH32M,
                "",
                { 0x6F },
                "tb",
                0xEF,
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
                0xEF,
                false,
                nullptr,
                nullptr,
            },
        },
    };

    void onNetworkChanged() override;

private:
    void initNode(std::uint32_t index = 0) override;
    [[nodiscard]] std::string generateTaprootAddress() const;
    [[nodiscard]] std::string generateP2WPKHAddress() const;

    std::vector<unsigned char> createP2WPKHScriptPubKey() const;
    std::vector<unsigned char> createTaprootScriptPubKey() const;

    [[nodiscard]] std::array<uint8_t, 32> bip86Tweak(
        const std::array<uint8_t, 33>& pubkey33) const;

    static std::array<uint8_t, 32> sha256(const uint8_t* data, size_t len);

    static constexpr std::size_t PUBLIC_KEY_SIZE = 33;
    static constexpr std::size_t SCHNORR_PUBLIC_KEY_SIZE = 32;

private:
    bool useTaproot_ { true };
};

}
#endif // BITCOINWALLET_H
