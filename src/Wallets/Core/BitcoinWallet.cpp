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

#include "BitcoinWallet.h"

#include "Context.h"

namespace Daitengu::Wallets {

BitcoinWallet::BitcoinWallet(bool useTaproot, Network::Type network)
    : ChainWallet(ChainType::BITCOIN, network)
    , useTaproot_(useTaproot)
{
}

void BitcoinWallet::fromPrivateKey(const std::string& privateKey)
{
    std::memset(&node_, 0, sizeof(HDNode));
    seed_.clear();
    mnemonic_.clear();

    std::vector<unsigned char> decoded;
    if (!DecodeBase58(privateKey, decoded, 64)) {
        throw std::invalid_argument("WIF base58 decode failed");
    }

    if (decoded.size() != 38) {
        throw std::invalid_argument("WIF must be 38 bytes (version+32-byte "
                                    "key+compression+4-byte checksum)");
    }

    std::vector<unsigned char> payload(decoded.begin(), decoded.begin() + 34);

    std::array<unsigned char, 32> hash1, hash2;
    sha256_Raw(payload.data(), payload.size(), hash1.data());
    sha256_Raw(hash1.data(), hash1.size(), hash2.data());
    // crypto_hash_sha256(hash1.data(), payload.data(), payload.size());
    // crypto_hash_sha256(hash2.data(), hash1.data(), hash1.size());

    if (hash2[0] != decoded[34] || hash2[1] != decoded[35]
        || hash2[2] != decoded[36] || hash2[3] != decoded[37]) {
        throw std::invalid_argument("WIF checksum mismatch");
    }

    uint8_t versionByte = payload[0];
    if (versionByte != 0x80) {
        throw std::invalid_argument("Unexpected version byte (not 0x80?)");
    }

    if (payload[33] != 0x01) {
        throw std::invalid_argument("No compression flag in WIF");
    }

    std::memcpy(node_.private_key, &payload[1], 32);

    node_.curve = get_curve_by_name("secp256k1");
    if (!node_.curve || !node_.curve->params) {
        throw std::runtime_error("Failed to set node_.curve = secp256k1_info");
    }

    node_.is_public_key_set = false;
    int err = hdnode_fill_public_key(&node_);
    if (err != 0) {
        throw std::runtime_error(
            "hdnode_fill_public_key failed (invalid private key?)");
    }
}

std::string BitcoinWallet::getAddress(std::uint32_t index)
{
    if (!mnemonic_.empty())
        initNode(index);

    if (useTaproot_) {
        return generateTaprootAddress();
    } else {
        return generateP2WPKHAddress();
    }
}

std::string BitcoinWallet::getPrivateKey(std::uint32_t index)
{
    if (!mnemonic_.empty())
        initNode(index);

    std::vector<unsigned char> payload(34);
    payload[0] = 0x80;
    std::memcpy(&payload[1], node_.private_key, 32);
    payload[33] = 0x01;

    std::array<unsigned char, 32> hash1, hash2;
    sha256_Raw(payload.data(), payload.size(), hash1.data());
    sha256_Raw(hash1.data(), hash1.size(), hash2.data());
    // crypto_hash_sha256(hash1.data(), payload.data(), payload.size());
    // crypto_hash_sha256(hash2.data(), hash1.data(), hash1.size());

    std::vector<unsigned char> fullData;
    fullData.reserve(34 + 4);
    fullData.insert(fullData.end(), payload.begin(), payload.end());
    fullData.push_back(hash2[0]);
    fullData.push_back(hash2[1]);
    fullData.push_back(hash2[2]);
    fullData.push_back(hash2[3]);

    return EncodeBase58(fullData);
}

BaseWallet::KeyPair BitcoinWallet::deriveKeyPair(std::uint32_t index)
{
    return KeyPair {
        getAddress(index),
        getPrivateKey(index),
    };
}

void BitcoinWallet::onNetworkChanged()
{
}

void BitcoinWallet::initNode(std::uint32_t index)
{
    if (hdnode_from_seed(seed_.data(), SEED_SIZE, "secp256k1", &node_) != 1) {
        throw std::runtime_error("Failed to create HD node from seed.");
    }

    uint32_t purpose = useTaproot_ ? 86 : 44;

    std::uint32_t path[] = {
        purpose | HARDENED, // purpose: BIP86 (Taproot)
        0 | HARDENED,       // coin type: Bitcoin
        0 | HARDENED,       // account
        0,                  // change (external chain)
        index               // address index
    };

    if (hdnode_private_ckd_cached(
            &node_, path, sizeof(path) / sizeof(std::uint32_t), nullptr)
        != 1) {
        throw std::runtime_error("Failed to derive key path.");
    }

    if (hdnode_fill_public_key(&node_) != 0) {
        throw std::runtime_error("Failed to fill pubkey");
    }
}

std::string BitcoinWallet::generateTaprootAddress() const
{
    std::array<unsigned char, PUBLIC_KEY_SIZE> pubkey33;
    std::memcpy(pubkey33.data(), node_.public_key, PUBLIC_KEY_SIZE);

    std::array<uint8_t, 32> tweakedXonly = bip86Tweak(pubkey33);

    std::string hrp = "bc";

    char out[128] = { 0 };
    int ok = segwit_addr_encode(
        out, hrp.c_str(), 1, tweakedXonly.data(), tweakedXonly.size());
    if (!ok) {
        throw std::runtime_error(
            "Failed to encode taproot address via segwit_addr_encode");
    }
    return std::string(out);
}

std::string BitcoinWallet::generateP2WPKHAddress() const
{
    unsigned char h1[32], h2[20];
    sha256_Raw(node_.public_key, 33, h1);

    ripemd160(h1, 32, h2);

    char addr[128] = { 0 };
    const char* hrp = "bc";
    if (!segwit_addr_encode(addr, hrp, 0, h2, 20)) {
        throw std::runtime_error("segwit_addr_encode fail for p2wpkh");
    }

    return std::string(addr);
}

std::vector<uint8_t> BitcoinWallet::bip86TweakXOnlyPubkey(
    const uint8_t pubkey33[33]) const
{
    return std::vector<uint8_t>();
}

std::array<uint8_t, 32> BitcoinWallet::bip86Tweak(
    const std::array<uint8_t, 33>& pubkey33) const
{
    secp256k1_context* secp_ctx = getSecpContext();

    secp256k1_pubkey pubkey;
    if (!secp256k1_ec_pubkey_parse(
            secp_ctx, &pubkey, pubkey33.data(), pubkey33.size())) {
        throw std::runtime_error("Invalid secp256k1_pubkey in bip86Tweak");
    }

    std::array<uint8_t, 32> xOnly;
    std::memcpy(xOnly.data(), pubkey33.data() + 1, 32);

    static const char* TAPTWEAK_TAG = "TapTweak";
    auto taghash = sha256(reinterpret_cast<const uint8_t*>(TAPTWEAK_TAG),
        std::strlen(TAPTWEAK_TAG));

    std::vector<uint8_t> buf;
    buf.insert(buf.end(), taghash.begin(), taghash.end());
    buf.insert(buf.end(), taghash.begin(), taghash.end());
    buf.insert(buf.end(), xOnly.begin(), xOnly.end());

    auto tweak = sha256(buf.data(), buf.size());

    secp256k1_xonly_pubkey xpub;
    int pk_parity = 0;
    if (!secp256k1_xonly_pubkey_from_pubkey(
            secp_ctx, &xpub, &pk_parity, &pubkey)) {
        throw std::runtime_error("Fail xonly_pubkey_from_pubkey");
    }

    secp256k1_pubkey pubkey_tweaked;
    if (!secp256k1_xonly_pubkey_tweak_add(
            secp_ctx, &pubkey_tweaked, &xpub, tweak.data())) {
        throw std::runtime_error(
            "Fail xonly_pubkey_tweak_add (maybe invalid tweak?)");
    }

    secp256k1_xonly_pubkey xpub_tweaked;
    if (!secp256k1_xonly_pubkey_from_pubkey(
            secp_ctx, &xpub_tweaked, nullptr, &pubkey_tweaked)) {
        throw std::runtime_error(
            "Fail xonly_pubkey_from_pubkey on tweaked pub");
    }

    std::array<uint8_t, 32> out32;
    if (!secp256k1_xonly_pubkey_serialize(
            secp_ctx, out32.data(), &xpub_tweaked)) {
        throw std::runtime_error("Fail xonly_pubkey_serialize");
    }

    return out32;
}

std::array<uint8_t, 32> BitcoinWallet::sha256(const uint8_t* data, size_t len)
{
    std::array<uint8_t, 32> hash {};
    sha256_Raw(data, len, hash.data());
    return hash;
}

}
