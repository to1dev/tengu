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

#include <algorithm>
#include <array>
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
#include "izanagi/secp256k1/secp256k1_musig.h"
#include "izanagi/secp256k1/secp256k1_schnorrsig.h"

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

enum class AddressType {
    P2PKH,          // Legacy P2PKH (starts with '1')
    P2SH,           // Legacy P2SH (starts with '3')
    P2WPKH,         // SegWit P2WPKH (starts with 'bc1q')
    P2WSH,          // SegWit P2WSH (starts with 'bc1q', multisig)
    P2SH_P2WSH,     // Nested SegWit P2SH-P2WSH (starts with '3', multisig)
    Taproot,        // Taproot P2TR (starts with 'bc1p')
    TaprootMultiSig // Taproot multisig (starts with 'bc1p')
};

struct TaprootScript {
    std::vector<unsigned char> script;
    uint8_t version = 0xC0;
};

struct TaprootScriptTree {
    std::vector<TaprootScript> leaves;
    uint32_t depth = 0;
};

class BitcoinWallet : public ChainWallet {
public:
    static inline constexpr std::string_view DEFAULT_DERIVATION_PATH
        = "m/86'/0'/0'";

    std::string_view getDerivationPath() const override
    {
        return DEFAULT_DERIVATION_PATH;
    }

    explicit BitcoinWallet(
        bool useTaproot = true, Network::Type network = Network::Type::MAINNET);

    static bool isValid(std::string_view address);

    void fromPrivateKey(const std::string& privateKey) override;
    [[nodiscard]] std::string getAddress(std::uint32_t index = 0) override;
    [[nodiscard]] std::string getExtendedAddress(std::uint32_t index = 0,
        AddressType type = AddressType::Taproot,
        const std::vector<std::array<unsigned char, 33>>& pubkeys = {},
        uint8_t m = 0, const TaprootScriptTree& script_tree = {});
    [[nodiscard]] std::string getPrivateKey(std::uint32_t index = 0) override;
    [[nodiscard]] KeyPair deriveKeyPair(std::uint32_t index = 0) override;

    [[nodiscard]] std::string getScriptPubKey(std::uint32_t index = 0);
    [[nodiscard]] std::string getScriptHash(std::uint32_t index = 0);
    [[nodiscard]] std::string getP2PKScriptHex(std::uint32_t index = 0);

    [[nodiscard]] std::array<unsigned char, 32> generateMuSig2AggregatePubkey(
        const std::vector<std::array<unsigned char, 33>>& pubkeys,
        secp256k1_musig_keyagg_cache* keyagg_cache = nullptr) const;

    [[nodiscard]] bool aggregateMuSig2Nonces(
        const std::vector<secp256k1_musig_pubnonce>& pubnonces,
        secp256k1_musig_aggnonce* aggnonce) const;

    [[nodiscard]] bool generateMuSig2Nonce(secp256k1_musig_secnonce* secnonce,
        secp256k1_musig_pubnonce* pubnonce,
        const std::array<unsigned char, 32>& seckey,
        const std::array<unsigned char, 33>& pubkey,
        const std::vector<unsigned char>& message,
        const secp256k1_musig_keyagg_cache* keyagg_cache,
        const std::array<unsigned char, 32>& extra_input = {}) const;

    [[nodiscard]] std::vector<unsigned char> generateMuSig2PartialSignature(
        const std::array<unsigned char, 32>& seckey,
        const std::array<unsigned char, 33>& pubkey,
        secp256k1_musig_secnonce* secnonce,
        const std::vector<secp256k1_musig_pubnonce>& pubnonces,
        const secp256k1_musig_keyagg_cache* keyagg_cache,
        const std::vector<unsigned char>& message) const;

    [[nodiscard]] std::vector<unsigned char> aggregateMuSig2Signatures(
        const std::vector<std::vector<unsigned char>>& partial_sigs,
        const std::vector<secp256k1_musig_pubnonce>& pubnonces,
        const std::vector<unsigned char>& message,
        const secp256k1_musig_keyagg_cache* keyagg_cache) const;

    [[nodiscard]] bool verifyMuSig2PartialSignature(
        const std::vector<unsigned char>& partial_sig,
        const std::array<unsigned char, 33>& pubkey,
        const secp256k1_musig_pubnonce* pubnonce,
        const std::vector<secp256k1_musig_pubnonce>& pubnonces,
        const secp256k1_musig_keyagg_cache* keyagg_cache,
        const std::vector<unsigned char>& message) const;

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

    [[nodiscard]] std::string generateP2PKHAddress() const;
    [[nodiscard]] std::string generateP2SHAddress(
        const std::vector<unsigned char>& redeemScript) const;
    [[nodiscard]] std::string generateP2WSHAddress(
        const std::vector<std::array<unsigned char, 33>>& pubkeys,
        uint8_t m) const;
    [[nodiscard]] std::string generateP2SHP2WSHAddress(
        const std::vector<std::array<unsigned char, 33>>& pubkeys,
        uint8_t m) const;
    [[nodiscard]] std::string generateTaprootMultiSigAddress(
        const std::vector<std::array<unsigned char, 33>>& pubkeys, uint8_t m,
        const TaprootScriptTree& script_tree) const;

    [[nodiscard]] std::vector<unsigned char> createP2WPKHScriptPubKey() const;
    [[nodiscard]] std::vector<unsigned char> createTaprootScriptPubKey() const;

    [[nodiscard]] std::vector<unsigned char> createP2PKScript() const;
    [[nodiscard]] std::vector<unsigned char> createMultiSigRedeemScript(
        const std::vector<std::array<unsigned char, 33>>& pubkeys,
        uint8_t m) const;

    [[nodiscard]] std::array<uint8_t, 32> computeTapLeafHash(
        const TaprootScript& leaf) const;
    [[nodiscard]] std::array<uint8_t, 32> computeTapBranchHash(
        const std::array<uint8_t, 32>& left,
        const std::array<uint8_t, 32>& right) const;
    [[nodiscard]] std::array<uint8_t, 32> computeMerkleRoot(
        const TaprootScriptTree& tree) const;

    [[nodiscard]] std::array<uint8_t, 32> bip86Tweak(
        const std::array<uint8_t, 33>& pubkey33) const;

    static std::array<uint8_t, 32> sha256(const uint8_t* data, size_t len);

    [[nodiscard]] secp256k1_pubkey parsePubkey(
        const std::array<unsigned char, 33>& pubkey) const;

    static constexpr std::size_t PUBLIC_KEY_SIZE = 33;
    static constexpr std::size_t SCHNORR_PUBLIC_KEY_SIZE = 32;

private:
    bool useTaproot_ { true };
};

}
#endif // BITCOINWALLET_H
