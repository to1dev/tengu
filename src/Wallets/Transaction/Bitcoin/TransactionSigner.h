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
#include <vector>

#include "../../Core/BitcoinWallet.h"

using namespace Daitengu::Wallets;

#include "Transaction.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "secp256k1.h"
#include "secp256k1_musig.h"
#include "secp256k1_schnorrsig.h"

#ifdef __cplusplus
}
#endif

namespace bitcoin {

// Structure representing a Taproot script tree for script-path spending
struct TaprootScriptTree {
    std::vector<unsigned char> script; // Script to execute
    std::vector<std::vector<unsigned char>>
        merkle_path;                   // Merkle path to the root
    std::array<unsigned char, 32>
        taproot_output_key;            // Taproot output public key (tweaked)
};

class TransactionSigner {
public:
    explicit TransactionSigner(secp256k1_context* ctx, BitcoinWallet* wallet);

    // Sign transaction input with ECDSA (for P2PKH, P2WPKH)
    [[nodiscard]] std::vector<unsigned char> signECDSA(const Transaction& tx,
        size_t input_index, const std::array<unsigned char, 32>& private_key,
        const std::vector<unsigned char>& script_code, uint64_t value,
        uint8_t sighash_type = 0x01) const;

    // Sign transaction input with Schnorr (for Taproot)
    [[nodiscard]] std::vector<unsigned char> signSchnorr(const Transaction& tx,
        size_t input_index, const std::array<unsigned char, 32>& private_key,
        const std::vector<unsigned char>& script_code, uint64_t value,
        uint8_t sighash_type = 0x01) const;

    // Sign transaction input with MuSig2 (for Taproot multisig)
    [[nodiscard]] std::vector<unsigned char> signMuSig2(const Transaction& tx,
        size_t input_index, const std::array<unsigned char, 32>& private_key,
        const std::array<unsigned char, 33>& public_key,
        const std::vector<std::array<unsigned char, 33>>& all_pubkeys,
        secp256k1_musig_secnonce* secnonce, secp256k1_musig_pubnonce* pubnonce,
        const std::vector<secp256k1_musig_pubnonce>& all_pubnonces,
        secp256k1_musig_keyagg_cache* keyagg_cache,
        uint8_t sighash_type = 0x01) const;

    // Aggregates MuSig2 partial signatures into a final signature
    // Matches BitcoinWallet's aggregateMuSig2Signatures declaration
    [[nodiscard]] std::vector<unsigned char> aggregateMuSig2Signatures(
        const std::vector<std::vector<unsigned char>>& partial_sigs,
        const std::vector<secp256k1_musig_pubnonce>& pubnonces,
        const std::vector<unsigned char>& message,
        const secp256k1_musig_keyagg_cache* keyagg_cache) const;

    // Signs a transaction input with a custom script (for P2SH, P2WSH, HTLC)
    [[nodiscard]] std::vector<unsigned char> signCustomScript(
        const Transaction& tx, size_t input_index,
        const std::array<unsigned char, 32>& private_key,
        const std::vector<unsigned char>& script_code, uint64_t value,
        uint8_t sighash_type, const std::vector<unsigned char>& custom_script,
        bool is_schnorr = false) const;

    // Signs a transaction input for Taproot script-path spending
    [[nodiscard]] std::vector<unsigned char> signTaprootScript(
        const Transaction& tx, size_t input_index,
        const std::array<unsigned char, 32>& private_key,
        const TaprootScriptTree& script_tree, uint64_t value,
        uint8_t sighash_type, bool is_schnorr = true) const;

private:
    secp256k1_context* ctx_;
    BitcoinWallet* wallet_;
};
}
