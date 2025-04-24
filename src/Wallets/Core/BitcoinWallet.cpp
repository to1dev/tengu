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

#include "../Utils/Hex.hpp"

namespace Daitengu::Wallets {

#include <cstdint>
#include <stdexcept>
#include <vector>

static inline std::vector<uint8_t> convertbits(
    const std::vector<uint8_t>& in, int fromBits, int toBits, bool pad)
{
    if (fromBits < 1 || fromBits > 8)
        throw std::invalid_argument("invalid fromBits");
    if (toBits < 1 || toBits > 8)
        throw std::invalid_argument("invalid toBits");

    std::vector<uint8_t> out;
    out.reserve((in.size() * fromBits + toBits - 1) / toBits);

    uint32_t value = 0;
    int bits = 0;

    const uint32_t maxv = (1 << toBits) - 1;

    for (auto c : in) {
        if (c >> fromBits) {
            throw std::invalid_argument("convertbits: invalid input data");
        }
        value = (value << fromBits) | c;
        bits += fromBits;
        while (bits >= toBits) {
            bits -= toBits;
            out.push_back((value >> bits) & maxv);
        }
    }

    if (pad) {
        if (bits > 0) {
            out.push_back((value << (toBits - bits)) & maxv);
        }
    } else {
        if (bits > 0) {
            if ((value << (toBits - bits)) & maxv) {
                throw std::invalid_argument(
                    "convertbits: non-zero padding in decode");
            }
        }
    }

    return out;
}

static inline std::vector<unsigned char> doubleSha256(
    const std::vector<unsigned char>& data)
{
    uint8_t digest1[32];
    sha256_Raw(data.data(), data.size(), digest1);

    uint8_t digest2[32];
    sha256_Raw(digest1, 32, digest2);

    return std::vector<unsigned char>(digest2, digest2 + 32);
}

static inline bool isValidBase58Address(std::string_view address)
{
    std::string addrStr(address);

    std::vector<unsigned char> decoded;
    if (!DecodeBase58(addrStr, decoded, 100)) {
        return false;
    }

    if (decoded.size() < 5) {
        return false;
    }

    size_t checksumPos = decoded.size() - 4;
    std::vector<unsigned char> payload(
        decoded.begin(), decoded.begin() + checksumPos);
    std::vector<unsigned char> givenChecksum(
        decoded.begin() + checksumPos, decoded.end());

    std::vector<unsigned char> hash = doubleSha256(payload);
    if (!std::equal(hash.begin(), hash.begin() + 4, givenChecksum.begin())) {
        return false;
    }

    uint8_t version = payload[0];
    std::vector<unsigned char> keyHash(payload.begin() + 1, payload.end());

    if (version == 0x00) {
        return (keyHash.size() == 20);
    } else if (version == 0x05) {
        return (keyHash.size() == 20);
    } else {
        return false;
    }
}

static inline bool isValidBech32Address(std::string_view address)
{
    if (address.empty()) {
        return false;
    }

    std::string input(address);

    char hrp[84] = { 0 };
    uint8_t data[82] = { 0 };
    size_t data_len = sizeof(data);

    bech32_encoding enc = bech32_decode(hrp, data, &data_len, input.c_str());
    if (enc == BECH32_ENCODING_NONE) {
        return false;
    }

    if (std::strcmp(hrp, "bc") != 0) {
        return false;
    }

    if (data_len < 1) {
        return false;
    }

    uint8_t witver = data[0];
    if (witver > 16) {
        return false;
    }

    std::vector<uint8_t> program_5bit(data + 1, data + data_len);

    std::vector<uint8_t> program_8bit;
    try {
        program_8bit = convertbits(program_5bit, 5, 8, false);
    } catch (...) {
        return false;
    }

    size_t prog_len = program_8bit.size();
    if (prog_len < 2 || prog_len > 40) {
        return false;
    }

    if (witver == 0) {
        if (enc != BECH32_ENCODING_BECH32) {
            return false;
        }
        if (prog_len == 20 || prog_len == 32) {
            return true;
        }
        return false;
    } else if (witver == 1) {
        if (enc != BECH32_ENCODING_BECH32M) {
            return false;
        }
        if (prog_len == 32) {
            return true;
        }
        return false;
    } else {
        if (enc != BECH32_ENCODING_BECH32M) {
            return false;
        }
        return true;
    }
}

BitcoinWallet::BitcoinWallet(AddressType addressType, Network::Type network)
    : ChainWallet(ChainType::BITCOIN, network)
    , addressType_(addressType)
{
}

bool BitcoinWallet::isValid(std::string_view address)
{
    if (address.empty()) {
        return false;
    }

    if (address.size() >= 3 && address[0] == 'b' && address[1] == 'c'
        && address[2] == '1') {
        if (isValidBech32Address(address)) {
            return true;
        }
        return false;
    }

    char first = address.front();
    if (first == '1' || first == '3') {
        return isValidBase58Address(address);
    }

    return false;
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
    if (hdnode_fill_public_key(&node_) != 0) {
        throw std::runtime_error(
            "hdnode_fill_public_key failed (invalid private key?)");
    }
}

std::string BitcoinWallet::getAddress(std::uint32_t index)
{
    if (!mnemonic_.empty())
        initNode(index);

    switch (addressType_) {
    case AddressType::Taproot:
        return generateTaprootAddress();

    case AddressType::P2WPKH:
        return generateP2WPKHAddress();

    default:
        break;
    }

    return std::string();
}

std::string BitcoinWallet::getExtendedAddress(std::uint32_t index,
    AddressType type, const std::vector<std::array<unsigned char, 33>>& pubkeys,
    uint8_t m, const TaprootScriptTree& script_tree)
{
    addressType_ = type;

    if (!mnemonic_.empty())
        initNode(index);

    switch (type) {
    case AddressType::P2PKH:
        return generateP2PKHAddress();

    case AddressType::P2SH: {
        if (pubkeys.empty() || m == 0) {
            throw std::invalid_argument("P2SH requires multisig params");
        }

        return generateP2SHAddress(createMultiSigRedeemScript(pubkeys, m));
    }

    case AddressType::P2WPKH:
        return generateP2WPKHAddress();

    case AddressType::P2WSH: {
        if (pubkeys.empty() || m == 0) {
            throw std::invalid_argument("P2WSH requires pubkeys and m");
        }

        return generateP2WSHAddress(pubkeys, m);
    }

    case AddressType::P2SH_P2WSH: {
        if (pubkeys.empty() || m == 0) {
            throw std::invalid_argument("P2SH-P2WSH requires pubkeys and m");
        }

        return generateP2SHP2WSHAddress(pubkeys, m);
    }

    case AddressType::Taproot:
        return generateTaprootAddress();

    case AddressType::TaprootMultiSig: {
        if (pubkeys.empty() || m == 0) {
            throw std::invalid_argument(
                "Taproot multisig requires pubkeys and m");
        }

        return generateTaprootMultiSigAddress(pubkeys, m, script_tree);
    }

    default:
        throw std::invalid_argument("Unsupported address type");
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

std::string BitcoinWallet::getScriptPubKey(std::uint32_t index)
{
    if (!mnemonic_.empty()) {
        initNode(index);
    }

    std::vector<unsigned char> script;
    switch (addressType_) {
    case AddressType::Taproot:
        script = createTaprootScriptPubKey();

    case AddressType::P2WPKH:
        script = createP2WPKHScriptPubKey();

    default:
        break;
    }

    return BytesToHex(script.data(), script.size());
}

std::string BitcoinWallet::getScriptHash(std::uint32_t index)
{
    std::string spkHex = getScriptPubKey(index);

    std::vector<unsigned char> spkBin;
    spkBin.reserve(spkHex.size() / 2);

    auto hex2bin = [&](char c) -> int {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        return -1;
    };
    for (size_t i = 0; i < spkHex.size(); i += 2) {
        int hi = hex2bin(spkHex[i]);
        int lo = hex2bin(spkHex[i + 1]);
        if (hi < 0 || lo < 0) {
            throw std::runtime_error("Invalid script hex");
        }
        spkBin.push_back((hi << 4) | lo);
    }

    auto hash = sha256(spkBin.data(), spkBin.size());

    return BytesToHex(hash.data(), hash.size());
}

std::string BitcoinWallet::getP2PKScriptHex(std::uint32_t index)
{
    if (!mnemonic_.empty()) {
        initNode(index);
    }

    auto script = createP2PKScript();
    return BytesToHex(script.data(), script.size());
}

std::array<unsigned char, 32> BitcoinWallet::generateMuSig2AggregatePubkey(
    const std::vector<std::array<unsigned char, 33>>& pubkeys,
    secp256k1_musig_keyagg_cache* keyagg_cache) const
{
    if (pubkeys.empty()) {
        throw std::invalid_argument("No public keys provided");
    }

    secp256k1_context* ctx = getSecpContext();
    std::vector<secp256k1_pubkey> pubkey_vec;
    std::vector<const secp256k1_pubkey*> pubkey_ptrs;

    // Convert and validate public keys
    pubkey_vec.reserve(pubkeys.size());
    pubkey_ptrs.reserve(pubkeys.size());
    for (const auto& pk : pubkeys) {
        pubkey_vec.push_back(parsePubkey(pk));
        pubkey_ptrs.push_back(&pubkey_vec.back());
    }

    // Sort public keys for consistency
    if (!secp256k1_ec_pubkey_sort(
            ctx, pubkey_ptrs.data(), pubkey_ptrs.size())) {
        throw std::runtime_error("Failed to sort public keys");
    }

    // Initialize MuSig2 key aggregation
    secp256k1_xonly_pubkey agg_pk;
    secp256k1_musig_keyagg_cache local_cache;
    if (!keyagg_cache) {
        keyagg_cache = &local_cache;
    }

    if (!secp256k1_musig_pubkey_agg(ctx, &agg_pk, keyagg_cache,
            pubkey_ptrs.data(), pubkey_ptrs.size())) {
        throw std::runtime_error("Failed to aggregate public keys");
    }

    // Serialize aggregated public key
    std::array<unsigned char, 32> agg_pk_serialized;
    if (!secp256k1_xonly_pubkey_serialize(
            ctx, agg_pk_serialized.data(), &agg_pk)) {
        throw std::runtime_error("Failed to serialize aggregated public key");
    }

    return agg_pk_serialized;
}

bool BitcoinWallet::aggregateMuSig2Nonces(
    const std::vector<secp256k1_musig_pubnonce>& pubnonces,
    secp256k1_musig_aggnonce* aggnonce) const
{
    if (pubnonces.empty()) {
        return false;
    }

    secp256k1_context* ctx = getSecpContext();
    std::vector<const secp256k1_musig_pubnonce*> nonce_ptrs;
    nonce_ptrs.reserve(pubnonces.size());
    for (const auto& nonce : pubnonces) {
        nonce_ptrs.push_back(&nonce);
    }

    return secp256k1_musig_nonce_agg(
        ctx, aggnonce, nonce_ptrs.data(), nonce_ptrs.size());
}

bool BitcoinWallet::generateMuSig2Nonce(secp256k1_musig_secnonce* secnonce,
    secp256k1_musig_pubnonce* pubnonce,
    const std::array<unsigned char, 32>& seckey,
    const std::array<unsigned char, 33>& pubkey,
    const std::vector<unsigned char>& message,
    const secp256k1_musig_keyagg_cache* keyagg_cache,
    const std::array<unsigned char, 32>& extra_input) const
{
    secp256k1_context* ctx = getSecpContext();

    // Validate private key
    if (!secp256k1_ec_seckey_verify(ctx, seckey.data())) {
        throw std::runtime_error("Invalid secret key");
    }

    // Parse public key
    secp256k1_pubkey pk = parsePubkey(pubkey);

    // Generate random session ID
    std::array<unsigned char, 32> session_secrand;
    randombytes_buf(session_secrand.data(), 32);

    // Generate nonce
    const unsigned char* msg_ptr = message.empty() ? nullptr : message.data();
    const unsigned char* extra_ptr
        = extra_input.empty() ? nullptr : extra_input.data();
    if (!secp256k1_musig_nonce_gen(ctx, secnonce, pubnonce,
            session_secrand.data(), seckey.data(), &pk, msg_ptr, keyagg_cache,
            extra_ptr)) {
        throw std::runtime_error("Failed to generate MuSig2 nonce");
    }

    // Clear session_secrand to prevent reuse
    sodium_memzero(session_secrand.data(), session_secrand.size());
    return true;
}

std::vector<unsigned char> BitcoinWallet::generateMuSig2PartialSignature(
    const std::array<unsigned char, 32>& seckey,
    const std::array<unsigned char, 33>& pubkey,
    secp256k1_musig_secnonce* secnonce,
    const std::vector<secp256k1_musig_pubnonce>& pubnonces,
    const secp256k1_musig_keyagg_cache* keyagg_cache,
    const std::vector<unsigned char>& message) const
{
    secp256k1_context* ctx = getSecpContext();

    // Validate inputs
    if (!secp256k1_ec_seckey_verify(ctx, seckey.data())) {
        throw std::runtime_error("Invalid secret key");
    }
    if (pubnonces.empty()) {
        throw std::invalid_argument("No public nonces provided");
    }
    if (!keyagg_cache) {
        throw std::invalid_argument("Key aggregation cache required");
    }
    if (message.empty()) {
        throw std::invalid_argument("Message required");
    }

    // Parse public key and create keypair
    secp256k1_pubkey pk = parsePubkey(pubkey);
    secp256k1_keypair keypair;
    if (!secp256k1_keypair_create(ctx, &keypair, seckey.data())) {
        throw std::runtime_error("Failed to create keypair");
    }

    // Aggregate nonces
    // Note: In a real application, pubnonces must be collected from all signers
    // via network communication (e.g., TCP, WebSocket) or file exchange.
    secp256k1_musig_aggnonce aggnonce;
    if (!aggregateMuSig2Nonces(pubnonces, &aggnonce)) {
        throw std::runtime_error("Failed to aggregate nonces");
    }

    // Create signing session
    secp256k1_musig_session session;
    if (!secp256k1_musig_nonce_process(
            ctx, &session, &aggnonce, message.data(), keyagg_cache)) {
        throw std::runtime_error("Failed to process nonce");
    }

    // Generate partial signature
    secp256k1_musig_partial_sig partial_sig;
    if (!secp256k1_musig_partial_sign(
            ctx, &partial_sig, secnonce, &keypair, keyagg_cache, &session)) {
        throw std::runtime_error("Failed to generate partial signature");
    }

    // Serialize partial signature
    std::array<unsigned char, 32> sig_serialized;
    if (!secp256k1_musig_partial_sig_serialize(
            ctx, sig_serialized.data(), &partial_sig)) {
        throw std::runtime_error("Failed to serialize partial signature");
    }

    // Clear secnonce to prevent reuse
    sodium_memzero(secnonce, sizeof(*secnonce));
    return std::vector<unsigned char>(
        sig_serialized.begin(), sig_serialized.end());
}

std::vector<unsigned char> BitcoinWallet::aggregateMuSig2Signatures(
    const std::vector<std::vector<unsigned char>>& partial_sigs,
    const std::vector<secp256k1_musig_pubnonce>& pubnonces,
    const std::vector<unsigned char>& message,
    const secp256k1_musig_keyagg_cache* keyagg_cache) const
{
    secp256k1_context* ctx = getSecpContext();

    // Validate inputs
    if (partial_sigs.empty()) {
        throw std::invalid_argument("No partial signatures provided");
    }
    if (pubnonces.empty()) {
        throw std::invalid_argument("No public nonces provided");
    }
    if (!keyagg_cache) {
        throw std::invalid_argument("Key aggregation cache required");
    }
    if (message.empty()) {
        throw std::invalid_argument("Message required");
    }

    // Parse partial signatures
    // Note: In a real application, partial_sigs must be collected from all
    // signers via network communication (e.g., TCP, WebSocket) or file
    // exchange.
    std::vector<secp256k1_musig_partial_sig> sigs;
    std::vector<const secp256k1_musig_partial_sig*> sig_ptrs;
    sigs.reserve(partial_sigs.size());
    sig_ptrs.reserve(partial_sigs.size());
    for (const auto& sig : partial_sigs) {
        if (sig.size() != 32) {
            throw std::runtime_error("Invalid partial signature size");
        }
        secp256k1_musig_partial_sig partial_sig;
        if (!secp256k1_musig_partial_sig_parse(ctx, &partial_sig, sig.data())) {
            throw std::runtime_error("Invalid partial signature");
        }
        sigs.push_back(partial_sig);
        sig_ptrs.push_back(&sigs.back());
    }

    // Aggregate nonces
    secp256k1_musig_aggnonce aggnonce;
    if (!aggregateMuSig2Nonces(pubnonces, &aggnonce)) {
        throw std::runtime_error("Failed to aggregate nonces");
    }

    // Create signing session
    secp256k1_musig_session session;
    if (!secp256k1_musig_nonce_process(
            ctx, &session, &aggnonce, message.data(), keyagg_cache)) {
        throw std::runtime_error("Failed to process nonce");
    }

    // Aggregate signatures
    std::array<unsigned char, 64> final_sig;
    if (!secp256k1_musig_partial_sig_agg(ctx, final_sig.data(), &session,
            sig_ptrs.data(), sig_ptrs.size())) {
        throw std::runtime_error("Failed to aggregate signatures");
    }

    return std::vector<unsigned char>(final_sig.begin(), final_sig.end());
}

bool BitcoinWallet::verifyMuSig2PartialSignature(
    const std::vector<unsigned char>& partial_sig,
    const std::array<unsigned char, 33>& pubkey,
    const secp256k1_musig_pubnonce* pubnonce,
    const std::vector<secp256k1_musig_pubnonce>& pubnonces,
    const secp256k1_musig_keyagg_cache* keyagg_cache,
    const std::vector<unsigned char>& message) const
{
    secp256k1_context* ctx = getSecpContext();

    // Validate inputs
    if (partial_sig.size() != 32) {
        return false;
    }
    if (!pubnonce || pubnonces.empty()) {
        return false;
    }
    if (!keyagg_cache) {
        return false;
    }
    if (message.empty()) {
        return false;
    }

    // Parse partial signature
    secp256k1_musig_partial_sig sig;
    if (!secp256k1_musig_partial_sig_parse(ctx, &sig, partial_sig.data())) {
        return false;
    }

    // Parse public key
    secp256k1_pubkey pk = parsePubkey(pubkey);

    // Aggregate nonces
    secp256k1_musig_aggnonce aggnonce;
    if (!aggregateMuSig2Nonces(pubnonces, &aggnonce)) {
        return false;
    }

    // Create signing session
    secp256k1_musig_session session;
    if (!secp256k1_musig_nonce_process(
            ctx, &session, &aggnonce, message.data(), keyagg_cache)) {
        return false;
    }

    // Verify partial signature
    return secp256k1_musig_partial_sig_verify(
        ctx, &sig, pubnonce, &pk, keyagg_cache, &session);
}

void BitcoinWallet::onNetworkChanged()
{
}

void BitcoinWallet::initNode(std::uint32_t index)
{
    if (hdnode_from_seed(seed_.data(), SEED_SIZE, "secp256k1", &node_) != 1) {
        throw std::runtime_error("Failed to create HD node from seed.");
    }

    uint32_t purpose = 44; // BIP-44

    switch (addressType_) {
    case AddressType::P2PKH:
    case AddressType::P2SH:
        purpose = 44; // BIP-44
        break;
    case AddressType::P2SH_P2WSH:
        purpose = 49; // BIP-49
        break;
    case AddressType::P2WPKH:
    case AddressType::P2WSH:
        purpose = 84; // BIP-84
        break;
    case AddressType::Taproot:
    case AddressType::TaprootMultiSig:
        purpose = 86; // BIP-86
        break;
    default:
        break;
    }

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

    // std::string hrp = "bc";
    const char* hrp = (currentNetwork_ == Network::Type::MAINNET) ? "bc" : "tb";

    char out[128] = { 0 };
    if (!segwit_addr_encode(
            out, hrp, 1, tweakedXonly.data(), tweakedXonly.size())) {
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
    // const char* hrp = "bc";
    const char* hrp = (currentNetwork_ == Network::Type::MAINNET) ? "bc" : "tb";
    if (!segwit_addr_encode(addr, hrp, 0, h2, 20)) {
        throw std::runtime_error("segwit_addr_encode fail for p2wpkh");
    }

    return std::string(addr);
}

std::string BitcoinWallet::generateP2PKHAddress() const
{
    unsigned char sha256_hash[32];
    sha256_Raw(node_.public_key, 33, sha256_hash);

    unsigned char ripemd160_hash[20];
    ripemd160(sha256_hash, 32, ripemd160_hash);

    std::vector<unsigned char> payload;
    payload.reserve(1 + 20);
    uint8_t version_byte
        = (currentNetwork_ == Network::Type::MAINNET) ? 0x00 : 0x6F;
    payload.push_back(version_byte);
    payload.insert(payload.end(), ripemd160_hash, ripemd160_hash + 20);

    std::vector<unsigned char> checksum = doubleSha256(payload);

    std::vector<unsigned char> full_data;
    full_data.reserve(payload.size() + 4);
    full_data.insert(full_data.end(), payload.begin(), payload.end());
    full_data.insert(full_data.end(), checksum.begin(), checksum.begin() + 4);

    return EncodeBase58(full_data);
}

std::string BitcoinWallet::generateP2SHAddress(
    const std::vector<unsigned char>& redeemScript) const
{
    unsigned char sha256_hash[32];
    sha256_Raw(redeemScript.data(), redeemScript.size(), sha256_hash);

    unsigned char ripemd160_hash[20];
    ripemd160(sha256_hash, 32, ripemd160_hash);

    std::vector<unsigned char> payload;
    payload.reserve(1 + 20);
    uint8_t version_byte
        = (currentNetwork_ == Network::Type::MAINNET) ? 0x05 : 0xC4;
    payload.push_back(version_byte);
    payload.insert(payload.end(), ripemd160_hash, ripemd160_hash + 20);

    std::vector<unsigned char> checksum = doubleSha256(payload);

    std::vector<unsigned char> full_data;
    full_data.reserve(payload.size() + 4);
    full_data.insert(full_data.end(), payload.begin(), payload.end());
    full_data.insert(full_data.end(), checksum.begin(), checksum.begin() + 4);

    return EncodeBase58(full_data);
}

std::string BitcoinWallet::generateP2WSHAddress(
    const std::vector<std::array<unsigned char, 33>>& pubkeys, uint8_t m) const
{
    auto redeem_script = createMultiSigRedeemScript(pubkeys, m);

    std::array<unsigned char, 32> script_hash
        = sha256(redeem_script.data(), redeem_script.size());

    char addr[128] = { 0 };
    const char* hrp = (currentNetwork_ == Network::Type::MAINNET) ? "bc" : "tb";
    if (!segwit_addr_encode(addr, hrp, 0, script_hash.data(), 32)) {
        throw std::runtime_error("Failed to encode P2WSH address");
    }

    return std::string(addr);
}

std::string BitcoinWallet::generateP2SHP2WSHAddress(
    const std::vector<std::array<unsigned char, 33>>& pubkeys, uint8_t m) const
{
    auto redeem_script = createMultiSigRedeemScript(pubkeys, m);

    std::array<unsigned char, 32> script_hash
        = sha256(redeem_script.data(), redeem_script.size());

    std::vector<unsigned char> p2wsh_script;
    p2wsh_script.push_back(0x00); // OP_0
    p2wsh_script.push_back(0x20); // Push 32 bytes
    p2wsh_script.insert(
        p2wsh_script.end(), script_hash.begin(), script_hash.end());

    unsigned char sha256_hash[32];
    sha256_Raw(p2wsh_script.data(), p2wsh_script.size(), sha256_hash);
    unsigned char ripemd160_hash[20];
    ripemd160(sha256_hash, 32, ripemd160_hash);

    std::vector<unsigned char> payload;
    payload.reserve(1 + 20);
    uint8_t version_byte
        = (currentNetwork_ == Network::Type::MAINNET) ? 0x05 : 0xC4;
    payload.push_back(version_byte);
    payload.insert(payload.end(), ripemd160_hash, ripemd160_hash + 20);

    std::vector<unsigned char> checksum = doubleSha256(payload);

    std::vector<unsigned char> full_data;
    full_data.reserve(payload.size() + 4);
    full_data.insert(full_data.end(), payload.begin(), payload.end());
    full_data.insert(full_data.end(), checksum.begin(), checksum.begin() + 4);

    return EncodeBase58(full_data);
}

std::string BitcoinWallet::generateTaprootMultiSigAddress(
    const std::vector<std::array<unsigned char, 33>>& pubkeys, uint8_t m,
    const TaprootScriptTree& script_tree) const
{
    if (pubkeys.empty() || m == 0 || m > pubkeys.size()) {
        throw std::invalid_argument(
            "Invalid pubkeys or m for Taproot multisig");
    }

    secp256k1_context* secp_ctx = getSecpContext();

    // Generate MuSig2 aggregated public key
    secp256k1_musig_keyagg_cache keyagg_cache;
    auto agg_pubkey = generateMuSig2AggregatePubkey(pubkeys, &keyagg_cache);

    // Convert aggregated public key to x-only format
    secp256k1_xonly_pubkey internal_xonly;
    if (!secp256k1_xonly_pubkey_parse(
            secp_ctx, &internal_xonly, agg_pubkey.data())) {
        throw std::runtime_error("Failed to parse aggregated pubkey");
    }

    // Compute Merkle root from script tree
    auto merkle_root = computeMerkleRoot(script_tree);

    // Tweak the aggregated public key with the Merkle root
    secp256k1_pubkey tweaked_pubkey;
    if (!secp256k1_musig_pubkey_xonly_tweak_add(
            secp_ctx, &tweaked_pubkey, &keyagg_cache, merkle_root.data())) {
        throw std::runtime_error("Failed to tweak aggregated pubkey");
    }

    // Convert tweaked public key to x-only format
    secp256k1_xonly_pubkey tweaked_xonly;
    if (!secp256k1_xonly_pubkey_from_pubkey(
            secp_ctx, &tweaked_xonly, nullptr, &tweaked_pubkey)) {
        throw std::runtime_error("Failed to convert tweaked pubkey to x-only");
    }

    // Serialize tweaked public key
    std::array<uint8_t, 32> output_key;
    if (!secp256k1_xonly_pubkey_serialize(
            secp_ctx, output_key.data(), &tweaked_xonly)) {
        throw std::runtime_error("Failed to serialize tweaked pubkey");
    }

    // Encode as Bech32m address
    char addr[128] = { 0 };
    const char* hrp = (currentNetwork_ == Network::Type::MAINNET) ? "bc" : "tb";
    if (!segwit_addr_encode(addr, hrp, 1, output_key.data(), 32)) {
        throw std::runtime_error("Failed to encode Taproot multisig address");
    }

    return std::string(addr);
}

std::vector<unsigned char> BitcoinWallet::createP2WPKHScriptPubKey() const
{
    unsigned char h1[32], h2[20];
    sha256_Raw(node_.public_key, 33, h1);
    ripemd160(h1, 32, h2);

    std::vector<unsigned char> script;
    script.reserve(2 + 20);
    // push 0x00
    script.push_back(0x00); // OP_0
    // push length=0x14
    script.push_back(0x14); // 20
    // push 20 bytes
    script.insert(script.end(), h2, h2 + 20);

    return script;
}

std::vector<unsigned char> BitcoinWallet::createTaprootScriptPubKey() const
{
    std::array<unsigned char, PUBLIC_KEY_SIZE> pubkey33;
    std::memcpy(pubkey33.data(), node_.public_key, PUBLIC_KEY_SIZE);

    auto tweaked = bip86Tweak(pubkey33);

    std::vector<unsigned char> script;
    script.reserve(2 + 32);
    // OP_1 => 0x51
    script.push_back(0x51);
    // push length=0x20=32
    script.push_back(0x20);
    // push 32
    script.insert(script.end(), tweaked.begin(), tweaked.end());

    return script;
}

std::vector<unsigned char> BitcoinWallet::createP2PKScript() const
{
    if (!node_.is_public_key_set) {
        throw std::runtime_error("Public key not set");
    }

    std::vector<unsigned char> script;
    script.reserve(1 + 33 + 1);
    script.push_back(0x21); // Push 33 bytes (compressed pubkey)
    script.insert(script.end(), node_.public_key, node_.public_key + 33);
    script.push_back(0xAC); // OP_CHECKSIG
    return script;
}

std::vector<unsigned char> BitcoinWallet::createMultiSigRedeemScript(
    const std::vector<std::array<unsigned char, 33>>& pubkeys, uint8_t m) const
{
    if (m < 1 || m > pubkeys.size() || pubkeys.size() > 16) {
        throw std::invalid_argument("Invalid m or pubkeys size");
    }

    std::vector<std::array<unsigned char, 33>> sorted_pubkeys = pubkeys;
    std::sort(sorted_pubkeys.begin(), sorted_pubkeys.end());

    std::vector<unsigned char> script;
    script.push_back(0x50 + m);              // OP_m
    for (const auto& pubkey : sorted_pubkeys) {
        script.push_back(0x21);              // Push 33 bytes
        script.insert(script.end(), pubkey.begin(), pubkey.end());
    }
    script.push_back(0x50 + pubkeys.size()); // OP_n
    script.push_back(0xAE);                  // OP_CHECKMULTISIG

    return script;
}

std::array<uint8_t, 32> BitcoinWallet::computeTapLeafHash(
    const TaprootScript& leaf) const
{
    static const char* TAPLEAF_TAG = "TapLeaf";
    auto taghash = sha256(reinterpret_cast<const uint8_t*>(TAPLEAF_TAG),
        std::strlen(TAPLEAF_TAG));

    std::vector<uint8_t> buf;
    buf.insert(buf.end(), taghash.begin(), taghash.end());
    buf.insert(buf.end(), taghash.begin(), taghash.end());
    buf.push_back(leaf.version);
    buf.insert(buf.end(), leaf.script.begin(), leaf.script.end());

    return sha256(buf.data(), buf.size());
}

std::array<uint8_t, 32> BitcoinWallet::computeTapBranchHash(
    const std::array<uint8_t, 32>& left,
    const std::array<uint8_t, 32>& right) const
{
    static const char* TAPBRANCH_TAG = "TapBranch";
    auto taghash = sha256(reinterpret_cast<const uint8_t*>(TAPBRANCH_TAG),
        std::strlen(TAPBRANCH_TAG));

    std::vector<uint8_t> buf;
    buf.insert(buf.end(), taghash.begin(), taghash.end());
    buf.insert(buf.end(), taghash.begin(), taghash.end());

    if (std::lexicographical_compare(
            left.begin(), left.end(), right.begin(), right.end())) {
        buf.insert(buf.end(), left.begin(), left.end());
        buf.insert(buf.end(), right.begin(), right.end());
    } else {
        buf.insert(buf.end(), right.begin(), right.end());
        buf.insert(buf.end(), left.begin(), left.end());
    }

    return sha256(buf.data(), buf.size());
}

std::array<uint8_t, 32> BitcoinWallet::computeMerkleRoot(
    const TaprootScriptTree& tree) const
{
    if (tree.leaves.empty()) {
        return std::array<uint8_t, 32> {};
    }

    std::vector<std::array<uint8_t, 32>> hashes;
    for (const auto& leaf : tree.leaves) {
        hashes.push_back(computeTapLeafHash(leaf));
    }

    while (hashes.size() > 1) {
        std::vector<std::array<uint8_t, 32>> new_hashes;
        for (size_t i = 0; i < hashes.size(); i += 2) {
            if (i + 1 < hashes.size()) {
                new_hashes.push_back(
                    computeTapBranchHash(hashes[i], hashes[i + 1]));
            } else {
                new_hashes.push_back(hashes[i]);
            }
        }
        hashes = std::move(new_hashes);
    }

    return hashes[0];
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

secp256k1_pubkey BitcoinWallet::parsePubkey(
    const std::array<unsigned char, 33>& pubkey) const
{
    secp256k1_context* ctx = getSecpContext();
    secp256k1_pubkey pk;
    if (!secp256k1_ec_pubkey_parse(ctx, &pk, pubkey.data(), 33)) {
        throw std::runtime_error("Invalid public key");
    }

    return pk;
}
}

#ifdef __demo

int main()
{
    using namespace Daitengu::Wallets;

    // Initialize wallet
    BitcoinWallet wallet(true, Network::Type::TESTNET);

    // Example public keys (33-byte compressed format)
    std::vector<std::array<unsigned char, 33>> pubkeys
        = { // Replace with actual public keys (33 bytes each)
              { /* Dummy pubkey 1: fill with 33-byte compressed key */ },
              { /* Dummy pubkey 2: fill with 33-byte compressed key */ },
              { /* Dummy pubkey 3: fill with 33-byte compressed key */ }
          };
    std::array<unsigned char, 32> seckey = { /* 32-byte private key */ };
    std::array<unsigned char, 33> pubkey
        = { /* 33-byte public key corresponding to seckey */ };
    std::vector<unsigned char> message(32, 0); // Example 32-byte message hash

    // Step 1: Generate aggregated public key
    secp256k1_musig_keyagg_cache keyagg_cache;
    auto agg_pk = wallet.generateMuSig2AggregatePubkey(pubkeys, &keyagg_cache);

    // Step 2: Generate nonce (first round)
    secp256k1_musig_secnonce secnonce;
    secp256k1_musig_pubnonce pubnonce;
    wallet.generateMuSig2Nonce(
        &secnonce, &pubnonce, seckey, pubkey, message, &keyagg_cache);

    // Collect nonces from all signers (via network)
    std::vector<secp256k1_musig_pubnonce> pubnonces
        = { pubnonce /* more nonces */ };
    // TODO: Implement network communication to collect pubnonces
    // e.g., pubnonces = collectNonces(peer_addresses);

    // Step 3: Generate partial signature
    auto partial_sig = wallet.generateMuSig2PartialSignature(
        seckey, pubkey, &secnonce, pubnonces, &keyagg_cache, message);

    // Verify partial signature (optional)
    if (wallet.verifyMuSig2PartialSignature(partial_sig, pubkey, &pubnonce,
            pubnonces, &keyagg_cache, message)) {
        std::cout << "Partial signature verified\n";
    } else {
        std::cout << "Partial signature verification failed\n";
        return 1;
    }

    // Collect partial signatures from all signers (via network)
    std::vector<std::vector<unsigned char>> partial_sigs
        = { partial_sig /* more sigs */ };
    // TODO: Implement network communication to collect partial_sigs
    // e.g., partial_sigs = collectPartialSignatures(peer_addresses);

    // Step 4: Aggregate signatures
    auto final_sig = wallet.aggregateMuSig2Signatures(
        partial_sigs, pubnonces, message, &keyagg_cache);

    // Verify final signature
    secp256k1_context* ctx = getSecpContext();
    secp256k1_xonly_pubkey xonly_agg_pk;
    secp256k1_xonly_pubkey_parse(ctx, &xonly_agg_pk, agg_pk.data());
    if (secp256k1_schnorrsig_verify(ctx, final_sig.data(), message.data(),
            message.size(), &xonly_agg_pk)) {
        std::cout << "Final signature verified\n";
    } else {
        std::cout << "Final signature verification failed\n";
        return 1;
    }

    // Generate Taproot multisig address
    TaprootScriptTree script_tree; // Optional script tree
    uint8_t m = 2; // Example: 2-of-3 multisig (adjust as needed)
    std::string address = wallet.getExtendedAddress(
        0, AddressType::TaprootMultiSig, pubkeys, m, script_tree);
    std::cout << "Taproot multisig address: " << address << "\n";

    return 0;
}
#endif
