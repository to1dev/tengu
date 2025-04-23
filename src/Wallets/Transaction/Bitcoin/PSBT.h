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
#include <map>
#include <vector>

#include "Transaction.h"
#include "TransactionSigner.h"

namespace bitcoin {

struct PublicKey {
    std::array<unsigned char, 33> data; // 33-byte compressed public key

    explicit PublicKey(const std::array<unsigned char, 33>& pubkey_data)
        : data(pubkey_data)
    {
    }

    explicit PublicKey(const std::vector<unsigned char>& pubkey_data)
    {
        if (pubkey_data.size() != 33) {
            throw std::runtime_error("Invalid public key size");
        }
        std::copy(pubkey_data.begin(), pubkey_data.end(), data.begin());
    }

    bool operator<(const PublicKey& other) const
    {
        return std::memcmp(data.data(), other.data.data(), 33) < 0;
    }

    bool operator==(const PublicKey& other) const
    {
        return std::memcmp(data.data(), other.data.data(), 33) == 0;
    }
};

struct PSBTInput {
    std::vector<unsigned char> non_witness_utxo;
    std::vector<unsigned char> witness_utxo;
    std::vector<unsigned char> redeem_script;
    std::vector<unsigned char> witness_script;
    std::map<PublicKey, std::vector<unsigned char>> signatures;
    std::vector<secp256k1_musig_pubnonce> pubnonces;
};

struct PSBTOutput {
    std::vector<unsigned char> redeem_script;
    std::vector<unsigned char> witness_script;
};

class PSBT {
public:
    explicit PSBT(secp256k1_context* ctx);

    static PSBT create(const Transaction& tx, secp256k1_context* ctx);
    void addInputMetadata(size_t index, const PSBTInput& input);
    void addOutputMetadata(size_t index, const PSBTOutput& output);
    void signInput(size_t index,
        const std::array<unsigned char, 32>& private_key,
        const TransactionSigner& signer);
    void signInputMuSig2(size_t index,
        const std::array<unsigned char, 32>& private_key,
        const std::array<unsigned char, 33>& public_key,
        const std::vector<std::array<unsigned char, 33>>& all_pubkeys,
        secp256k1_musig_secnonce* secnonce, secp256k1_musig_pubnonce* pubnonce,
        const std::vector<secp256k1_musig_pubnonce>& all_pubnonces,
        secp256k1_musig_keyagg_cache* keyagg_cache,
        const TransactionSigner& signer);
    void signInputCustom(size_t index,
        const std::array<unsigned char, 32>& private_key,
        const std::vector<unsigned char>& custom_script, bool is_schnorr,
        const TransactionSigner& signer);
    void signInputTaprootScript(size_t index,
        const std::array<unsigned char, 32>& private_key,
        const TaprootScriptTree& script_tree, bool is_schnorr,
        const TransactionSigner& signer);
    void combine(const PSBT& other);
    [[nodiscard]] Transaction finalize() const;
    [[nodiscard]] std::vector<unsigned char> serialize() const;
    static PSBT parse(
        const std::vector<unsigned char>& raw, secp256k1_context* ctx);

private:
    static std::vector<unsigned char> encodeVarInt(uint64_t n);
    static uint64_t decodeVarInt(
        const unsigned char*& pos, const unsigned char* end);
    static void encodeKeyValue(std::vector<unsigned char>& result, uint8_t type,
        const std::vector<unsigned char>& key,
        const std::vector<unsigned char>& value);

    secp256k1_context* ctx_;
    Transaction tx_;
    std::vector<PSBTInput> inputs_;
    std::vector<PSBTOutput> outputs_;
};
}
