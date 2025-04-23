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

#include "TransactionSigner.h"

#include <sodium.h>

namespace bitcoin {

TransactionSigner::TransactionSigner(
    secp256k1_context* ctx, BitcoinWallet* wallet)
    : ctx_(ctx)
    , wallet_(wallet)
{
    if (!ctx_ || !wallet_) {
        throw std::invalid_argument("Invalid secp256k1 context or wallet");
    }
}

std::vector<unsigned char> TransactionSigner::signECDSA(const Transaction& tx,
    size_t input_index, const std::array<unsigned char, 32>& private_key,
    const std::vector<unsigned char>& script_code, uint64_t value,
    uint8_t sighash_type) const
{
    // Verify private key
    if (!secp256k1_ec_seckey_verify(ctx_, private_key.data())) {
        throw std::runtime_error("Invalid private key");
    }

    // Compute sighash
    auto sighash
        = tx.getSigHash(input_index, script_code, value, sighash_type, false);

    // Sign using ECDSA
    secp256k1_ecdsa_signature signature;
    if (!secp256k1_ecdsa_sign(ctx_, &signature, sighash.data(),
            private_key.data(), nullptr, nullptr)) {
        throw std::runtime_error("Failed to sign ECDSA");
    }

    // Serialize to DER format (PSBT expects DER)
    std::vector<unsigned char> der_signature(72);
    size_t der_len = der_signature.size();
    if (!secp256k1_ecdsa_signature_serialize_der(
            ctx_, der_signature.data(), &der_len, &signature)) {
        throw std::runtime_error("Failed to serialize ECDSA signature");
    }
    der_signature.resize(der_len);

    // Append sighash type
    der_signature.push_back(sighash_type);

    // Clear sensitive data
    sodium_memzero(&signature, sizeof(signature));

    return der_signature;
}

std::vector<unsigned char> TransactionSigner::signSchnorr(const Transaction& tx,
    size_t input_index, const std::array<unsigned char, 32>& private_key,
    const std::vector<unsigned char>& script_code, uint64_t value,
    uint8_t sighash_type) const
{
    // Verify private key
    if (!secp256k1_ec_seckey_verify(ctx_, private_key.data())) {
        throw std::runtime_error("Invalid private key");
    }

    // Compute sighash
    auto sighash
        = tx.getSigHash(input_index, script_code, value, sighash_type, true);

    secp256k1_keypair keypair;
    if (!secp256k1_keypair_create(ctx_, &keypair, private_key.data())) {
        throw std::runtime_error("Failed to create keypair");
    }

    // Sign using Schnorr
    std::array<unsigned char, 64> signature;
    if (!secp256k1_schnorrsig_sign32(
            ctx_, signature.data(), sighash.data(), &keypair, nullptr)) {
        throw std::runtime_error("Failed to sign Schnorr");
    }

    // Prepare result
    std::vector<unsigned char> result(signature.begin(), signature.end());
    if (sighash_type != 0x00) {
        result.push_back(sighash_type); // Append sighash type if not default
    }
    return result;
}

std::vector<unsigned char> TransactionSigner::signMuSig2(const Transaction& tx,
    size_t input_index, const std::array<unsigned char, 32>& private_key,
    const std::array<unsigned char, 33>& public_key,
    const std::vector<std::array<unsigned char, 33>>& all_pubkeys,
    secp256k1_musig_secnonce* secnonce, secp256k1_musig_pubnonce* pubnonce,
    const std::vector<secp256k1_musig_pubnonce>& all_pubnonces,
    secp256k1_musig_keyagg_cache* keyagg_cache, uint8_t sighash_type) const
{
    if (!secp256k1_ec_seckey_verify(ctx_, private_key.data())) {
        throw std::runtime_error("Invalid private key");
    }

    auto sighash = tx.getSigHash(input_index, {}, 0, sighash_type, true);

    secp256k1_musig_aggnonce aggnonce;
    std::vector<const secp256k1_musig_pubnonce*> nonce_ptrs;
    nonce_ptrs.reserve(all_pubnonces.size());
    for (const auto& nonce : all_pubnonces) {
        nonce_ptrs.push_back(&nonce);
    }

    if (!secp256k1_musig_nonce_agg(
            ctx_, &aggnonce, nonce_ptrs.data(), nonce_ptrs.size())) {
        throw std::runtime_error("Failed to aggregate nonces");
    }

    secp256k1_musig_session session;
    if (!secp256k1_musig_nonce_process(
            ctx_, &session, &aggnonce, sighash.data(), keyagg_cache)) {
        throw std::runtime_error("Failed to process nonce");
    }

    secp256k1_musig_partial_sig partial_sig;
    secp256k1_keypair keypair;
    if (!secp256k1_keypair_create(ctx_, &keypair, private_key.data())) {
        throw std::runtime_error("Failed to create keypair");
    }

    if (!secp256k1_musig_partial_sign(
            ctx_, &partial_sig, secnonce, &keypair, keyagg_cache, &session)) {
        throw std::runtime_error("Failed to generate partial signature");
    }

    std::array<unsigned char, 32> sig_serialized;
    if (!secp256k1_musig_partial_sig_serialize(
            ctx_, sig_serialized.data(), &partial_sig)) {
        throw std::runtime_error("Failed to serialize partial signature");
    }

    sodium_memzero(secnonce, sizeof(*secnonce));

    return std::vector<unsigned char>(
        sig_serialized.begin(), sig_serialized.end());
}

std::vector<unsigned char> TransactionSigner::aggregateMuSig2Signatures(
    const std::vector<std::vector<unsigned char>>& partial_sigs,
    const std::vector<secp256k1_musig_pubnonce>& pubnonces,
    const std::vector<unsigned char>& message,
    const secp256k1_musig_keyagg_cache* keyagg_cache) const
{
    return wallet_->aggregateMuSig2Signatures(
        partial_sigs, pubnonces, message, keyagg_cache);
}

std::vector<unsigned char> TransactionSigner::signCustomScript(
    const Transaction& tx, size_t input_index,
    const std::array<unsigned char, 32>& private_key,
    const std::vector<unsigned char>& script_code, uint64_t value,
    uint8_t sighash_type, const std::vector<unsigned char>& custom_script,
    bool is_schnorr) const
{
    if (!secp256k1_ec_seckey_verify(ctx_, private_key.data())) {
        throw std::runtime_error("Invalid private key");
    }

    auto sighash = tx.getSigHash(
        input_index, script_code, value, sighash_type, is_schnorr);

    secp256k1_keypair keypair;
    if (!secp256k1_keypair_create(ctx_, &keypair, private_key.data())) {
        throw std::runtime_error("Failed to create keypair");
    }

    std::vector<unsigned char> signature;
    if (is_schnorr) {
        std::array<unsigned char, 64> schnorr_sig;
        if (!secp256k1_schnorrsig_sign32(
                ctx_, schnorr_sig.data(), sighash.data(), &keypair, nullptr)) {
            throw std::runtime_error("Failed to sign Schnorr");
        }
        signature = std::vector<unsigned char>(
            schnorr_sig.begin(), schnorr_sig.end());
        if (sighash_type != 0x00) {
            signature.push_back(sighash_type);
        }
    } else {
        secp256k1_ecdsa_signature ecdsa_sig;
        if (!secp256k1_ecdsa_sign(ctx_, &ecdsa_sig, sighash.data(),
                private_key.data(), nullptr, nullptr)) {
            throw std::runtime_error("Failed to sign ECDSA");
        }
        signature.resize(72);
        size_t der_len = signature.size();
        if (!secp256k1_ecdsa_signature_serialize_der(
                ctx_, signature.data(), &der_len, &ecdsa_sig)) {
            throw std::runtime_error("Failed to serialize ECDSA signature");
        }
        signature.resize(der_len);
        signature.push_back(sighash_type);
        sodium_memzero(&ecdsa_sig, sizeof(ecdsa_sig));
    }

    return signature;
}

std::vector<unsigned char> TransactionSigner::signTaprootScript(
    const Transaction& tx, size_t input_index,
    const std::array<unsigned char, 32>& private_key,
    const TaprootScriptTree& script_tree, uint64_t value, uint8_t sighash_type,
    bool is_schnorr) const
{
    if (!secp256k1_ec_seckey_verify(ctx_, private_key.data())) {
        throw std::runtime_error("Invalid private key");
    }

    std::array<unsigned char, 32> leaf_hash;
    sha256_Raw(
        script_tree.script.data(), script_tree.script.size(), leaf_hash.data());
    std::array<unsigned char, 32> current_hash = leaf_hash;
    for (const auto& sibling : script_tree.merkle_path) {
        std::array<unsigned char, 64> combined;
        if (std::lexicographical_compare(current_hash.begin(),
                current_hash.end(), sibling.begin(), sibling.end())) {
            std::copy(
                current_hash.begin(), current_hash.end(), combined.begin());
            std::copy(sibling.begin(), sibling.end(), combined.begin() + 32);
        } else {
            std::copy(sibling.begin(), sibling.end(), combined.begin());
            std::copy(current_hash.begin(), current_hash.end(),
                combined.begin() + 32);
        }
        sha256_Raw(combined.data(), 64, current_hash.data());
    }
    if (current_hash != script_tree.taproot_output_key) {
        throw std::runtime_error("Invalid Taproot Merkle path");
    }

    auto sighash = tx.getSigHash(
        input_index, script_tree.script, value, sighash_type, true);

    secp256k1_keypair keypair;
    if (!secp256k1_keypair_create(ctx_, &keypair, private_key.data())) {
        throw std::runtime_error("Failed to create keypair");
    }

    std::vector<unsigned char> signature;
    if (is_schnorr) {
        std::array<unsigned char, 64> schnorr_sig;
        if (!secp256k1_schnorrsig_sign32(
                ctx_, schnorr_sig.data(), sighash.data(), &keypair, nullptr)) {
            throw std::runtime_error("Failed to sign Schnorr");
        }
        signature = std::vector<unsigned char>(
            schnorr_sig.begin(), schnorr_sig.end());
        if (sighash_type != 0x00) {
            signature.push_back(sighash_type);
        }
    } else {
        secp256k1_ecdsa_signature ecdsa_sig;
        if (!secp256k1_ecdsa_sign(ctx_, &ecdsa_sig, sighash.data(),
                private_key.data(), nullptr, nullptr)) {
            throw std::runtime_error("Failed to sign ECDSA");
        }
        signature.resize(72);
        size_t der_len = signature.size();
        if (!secp256k1_ecdsa_signature_serialize_der(
                ctx_, signature.data(), &der_len, &ecdsa_sig)) {
            throw std::runtime_error("Failed to serialize ECDSA signature");
        }
        signature.resize(der_len);
        signature.push_back(sighash_type);
        sodium_memzero(&ecdsa_sig, sizeof(ecdsa_sig));
    }

    std::vector<unsigned char> control_block = { 0x00 };
    for (const auto& sibling : script_tree.merkle_path) {
        control_block.insert(
            control_block.end(), sibling.begin(), sibling.end());
    }
    signature.insert(
        signature.end(), control_block.begin(), control_block.end());

    return signature;
}
}
