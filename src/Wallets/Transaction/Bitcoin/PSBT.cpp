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

#include "PSBT.h"

#include <sodium.h>

namespace bitcoin {

PSBT::PSBT(secp256k1_context* ctx)
    : ctx_(ctx)
    , tx_(2, 0)
{
    if (!ctx_) {
        throw std::invalid_argument("Invalid secp256k1 context");
    }
}

PSBT PSBT::create(const Transaction& tx, secp256k1_context* ctx)
{
    PSBT psbt(ctx);
    psbt.tx_ = tx;
    psbt.inputs_.resize(tx.inputs().size());
    psbt.outputs_.resize(tx.outputs().size());

    return psbt;
}

void PSBT::addInputMetadata(size_t index, const PSBTInput& input)
{
    if (index >= inputs_.size()) {
        throw std::invalid_argument("Invalid input index");
    }
    inputs_[index] = input;
}

void PSBT::addOutputMetadata(size_t index, const PSBTOutput& output)
{
    if (index >= outputs_.size()) {
        throw std::invalid_argument("Invalid output index");
    }
    outputs_[index] = output;
}

void PSBT::signInput(size_t index,
    const std::array<unsigned char, 32>& private_key,
    const TransactionSigner& signer)
{
    if (index >= inputs_.size()) {
        throw std::invalid_argument("Invalid input index");
    }

    auto& input = inputs_[index];
    std::vector<unsigned char> script_code = input.redeem_script.empty()
        ? input.witness_script
        : input.redeem_script;
    uint64_t value = 0;
    std::vector<unsigned char> signature;

    if (!input.witness_utxo.empty()) {
        signature
            = signer.signSchnorr(tx_, index, private_key, script_code, value);
    } else {
        signature
            = signer.signECDSA(tx_, index, private_key, script_code, value);
    }

    secp256k1_pubkey pubkey;
    if (!secp256k1_ec_pubkey_create(ctx_, &pubkey, private_key.data())) {
        throw std::runtime_error("Failed to derive public key");
    }
    std::array<unsigned char, 33> pubkey_data;
    size_t pubkey_len = 33;
    secp256k1_ec_pubkey_serialize(ctx_, pubkey_data.data(), &pubkey_len,
        &pubkey, SECP256K1_EC_COMPRESSED);
    input.signatures[PublicKey(pubkey_data)] = signature;
}

void PSBT::signInputMuSig2(size_t index,
    const std::array<unsigned char, 32>& private_key,
    const std::array<unsigned char, 33>& public_key,
    const std::vector<std::array<unsigned char, 33>>& all_pubkeys,
    secp256k1_musig_secnonce* secnonce, secp256k1_musig_pubnonce* pubnonce,
    const std::vector<secp256k1_musig_pubnonce>& all_pubnonces,
    secp256k1_musig_keyagg_cache* keyagg_cache, const TransactionSigner& signer)
{
    if (index >= inputs_.size()) {
        throw std::invalid_argument("Invalid input index");
    }

    auto& input = inputs_[index];
    auto signature = signer.signMuSig2(tx_, index, private_key, public_key,
        all_pubkeys, secnonce, pubnonce, all_pubnonces, keyagg_cache);
    input.signatures[PublicKey(public_key)] = signature;
    input.pubnonces = all_pubnonces;
}

void PSBT::signInputCustom(size_t index,
    const std::array<unsigned char, 32>& private_key,
    const std::vector<unsigned char>& custom_script, bool is_schnorr,
    const TransactionSigner& signer)
{
    if (index >= inputs_.size()) {
        throw std::invalid_argument("Invalid input index");
    }

    auto& input = inputs_[index];
    std::vector<unsigned char> script_code = input.redeem_script.empty()
        ? input.witness_script
        : input.redeem_script;
    uint64_t value = 0;
    auto signature = signer.signCustomScript(tx_, index, private_key,
        script_code, value, 0x01, custom_script, is_schnorr);

    secp256k1_pubkey pubkey;
    if (!secp256k1_ec_pubkey_create(ctx_, &pubkey, private_key.data())) {
        throw std::runtime_error("Failed to derive public key");
    }
    std::array<unsigned char, 33> pubkey_data;
    size_t pubkey_len = 33;
    secp256k1_ec_pubkey_serialize(ctx_, pubkey_data.data(), &pubkey_len,
        &pubkey, SECP256K1_EC_COMPRESSED);
    input.signatures[PublicKey(pubkey_data)] = signature;
}

void PSBT::signInputTaprootScript(size_t index,
    const std::array<unsigned char, 32>& private_key,
    const TaprootScriptTree& script_tree, bool is_schnorr,
    const TransactionSigner& signer)
{
    if (index >= inputs_.size()) {
        throw std::invalid_argument("Invalid input index");
    }

    auto& input = inputs_[index];
    uint64_t value = 0;
    auto signature = signer.signTaprootScript(
        tx_, index, private_key, script_tree, value, 0x01, is_schnorr);

    secp256k1_pubkey pubkey;
    if (!secp256k1_ec_pubkey_create(ctx_, &pubkey, private_key.data())) {
        throw std::runtime_error("Failed to derive public key");
    }
    std::array<unsigned char, 33> pubkey_data;
    size_t pubkey_len = 33;
    secp256k1_ec_pubkey_serialize(ctx_, pubkey_data.data(), &pubkey_len,
        &pubkey, SECP256K1_EC_COMPRESSED);
    input.signatures[PublicKey(pubkey_data)] = signature;
}

void PSBT::combine(const PSBT& other)
{
    if (tx_.serialize() != other.tx_.serialize()) {
        throw std::invalid_argument(
            "Cannot combine PSBTs with different transactions");
    }

    for (size_t i = 0; i < inputs_.size(); ++i) {
        auto& input = inputs_[i];
        const auto& other_input = other.inputs_[i];
        if (!other_input.non_witness_utxo.empty()
            && input.non_witness_utxo.empty()) {
            input.non_witness_utxo = other_input.non_witness_utxo;
        }
        if (!other_input.witness_utxo.empty() && input.witness_utxo.empty()) {
            input.witness_utxo = other_input.witness_utxo;
        }
        if (!other_input.redeem_script.empty() && input.redeem_script.empty()) {
            input.redeem_script = other_input.redeem_script;
        }
        if (!other_input.witness_script.empty()
            && input.witness_script.empty()) {
            input.witness_script = other_input.witness_script;
        }
        for (const auto& [pubkey, sig] : other_input.signatures) {
            input.signatures[pubkey] = sig;
        }
        input.pubnonces.insert(input.pubnonces.end(),
            other_input.pubnonces.begin(), other_input.pubnonces.end());
    }

    for (size_t i = 0; i < outputs_.size(); ++i) {
        auto& output = outputs_[i];
        const auto& other_output = other.outputs_[i];
        if (!other_output.redeem_script.empty()
            && output.redeem_script.empty()) {
            output.redeem_script = other_output.redeem_script;
        }
        if (!other_output.witness_script.empty()
            && output.witness_script.empty()) {
            output.witness_script = other_output.witness_script;
        }
    }
}

Transaction PSBT::finalize() const
{
    Transaction tx = tx_;
    for (size_t i = 0; i < inputs_.size(); ++i) {
        auto& input = tx.inputs()[i];
        const auto& psbt_input = inputs_[i];

        if (psbt_input.signatures.empty()) {
            throw std::runtime_error("Input not fully signed");
        }

        if (!psbt_input.witness_utxo.empty() && !psbt_input.pubnonces.empty()) {
            std::vector<std::vector<unsigned char>> partial_sigs;
            for (const auto& [pubkey, sig] : psbt_input.signatures) {
                partial_sigs.push_back(sig);
            }
            std::vector<unsigned char> script_code
                = psbt_input.witness_script.empty() ? psbt_input.redeem_script
                                                    : psbt_input.witness_script;
            uint64_t value = 0;
            auto sighash = tx.getSigHash(i, script_code, value, 0x01, true);
            TransactionSigner signer(ctx_, nullptr);
            auto aggregated_sig = signer.aggregateMuSig2Signatures(partial_sigs,
                psbt_input.pubnonces,
                std::vector<unsigned char>(sighash.begin(), sighash.end()),
                nullptr);
            input.witness = aggregated_sig;
            input.script_sig.clear();
        } else if (!psbt_input.witness_utxo.empty()) {
            std::vector<unsigned char> witness;
            for (const auto& [pubkey, sig] : psbt_input.signatures) {
                witness.insert(witness.end(), sig.begin(), sig.end());
            }
            input.witness = witness;
            input.script_sig.clear();
        } else {
            std::vector<unsigned char> script_sig;
            for (const auto& [pubkey, sig] : psbt_input.signatures) {
                script_sig.push_back(static_cast<unsigned char>(sig.size()));
                script_sig.insert(script_sig.end(), sig.begin(), sig.end());
                script_sig.push_back(
                    static_cast<unsigned char>(pubkey.data.size()));
                script_sig.insert(
                    script_sig.end(), pubkey.data.begin(), pubkey.data.end());
            }
            input.script_sig = script_sig;
        }
    }

    return tx;
}

std::vector<unsigned char> PSBT::serialize() const
{
    std::vector<unsigned char> result;
    result.insert(result.end(), { 0x70, 0x73, 0x62, 0x74, 0xFF });

    auto tx_data = tx_.serialize();
    encodeKeyValue(result, 0x00, {}, tx_data);
    result.push_back(0x00);

    for (size_t i = 0; i < inputs_.size(); ++i) {
        const auto& input = inputs_[i];
        if (!input.non_witness_utxo.empty()) {
            encodeKeyValue(result, 0x02, {}, input.non_witness_utxo);
        }
        if (!input.witness_utxo.empty()) {
            encodeKeyValue(result, 0x03, {}, input.witness_utxo);
        }
        if (!input.redeem_script.empty()) {
            encodeKeyValue(result, 0x04, {}, input.redeem_script);
        }
        if (!input.witness_script.empty()) {
            encodeKeyValue(result, 0x05, {}, input.witness_script);
        }
        for (const auto& [pubkey, sig] : input.signatures) {
            std::vector<unsigned char> pubkey_vec(
                pubkey.data.begin(), pubkey.data.end());
            encodeKeyValue(result, 0x06, pubkey_vec, sig);
        }
        result.push_back(0x00);
    }

    for (const auto& output : outputs_) {
        if (!output.redeem_script.empty()) {
            encodeKeyValue(result, 0x00, {}, output.redeem_script);
        }
        if (!output.witness_script.empty()) {
            encodeKeyValue(result, 0x01, {}, output.witness_script);
        }
        result.push_back(0x00);
    }

    return result;
}

PSBT PSBT::parse(const std::vector<unsigned char>& raw, secp256k1_context* ctx)
{
    PSBT psbt(ctx);
    const unsigned char* pos = raw.data();
    const unsigned char* end = raw.data() + raw.size();

    if (pos + 5 > end || pos[0] != 0x70 || pos[1] != 0x73 || pos[2] != 0x62
        || pos[3] != 0x74 || pos[4] != 0xFF) {
        throw std::runtime_error("Invalid PSBT magic bytes");
    }
    pos += 5;

    std::vector<unsigned char> tx_data;
    while (pos < end && *pos != 0x00) {
        auto key_len = decodeVarInt(pos, end);
        if (pos + key_len > end) {
            throw std::runtime_error("Invalid key length");
        }
        uint8_t type = *pos++;
        std::vector<unsigned char> key(pos, pos + key_len - 1);
        pos += key_len - 1;

        auto value_len = decodeVarInt(pos, end);
        if (pos + value_len > end) {
            throw std::runtime_error("Invalid value length");
        }
        std::vector<unsigned char> value(pos, pos + value_len);
        pos += value_len;

        if (type == 0x00 && key.empty()) {
            tx_data = value;
        }
    }
    if (*pos++ != 0x00) {
        throw std::runtime_error("Missing global separator");
    }

    Transaction tx(2, 0);
    TxInput input;
    input.txid.fill(0);
    input.vout = 0;
    input.sequence = 0xFFFFFFFF;
    tx.addInput(input);
    TxOutput output;
    output.value = 1000000;
    output.script_pubkey = { 0x00, 0x14 };
    tx.addOutput(output);
    psbt.tx_ = tx;

    psbt.inputs_.resize(psbt.tx_.inputs().size());
    for (size_t i = 0; i < psbt.inputs_.size() && pos < end; ++i) {
        auto& input = psbt.inputs_[i];
        while (pos < end && *pos != 0x00) {
            auto key_len = decodeVarInt(pos, end);
            if (pos + key_len > end) {
                throw std::runtime_error("Invalid key length");
            }
            uint8_t type = *pos++;
            std::vector<unsigned char> key(pos, pos + key_len - 1);
            pos += key_len - 1;

            auto value_len = decodeVarInt(pos, end);
            if (pos + value_len > end) {
                throw std::runtime_error("Invalid value length");
            }
            std::vector<unsigned char> value(pos, pos + value_len);
            pos += value_len;

            switch (type) {
            case 0x02:
                input.non_witness_utxo = value;
                break;
            case 0x03:
                input.witness_utxo = value;
                break;
            case 0x04:
                input.redeem_script = value;
                break;
            case 0x05:
                input.witness_script = value;
                break;
            case 0x06: {
                input.signatures[PublicKey(key)] = value;
                break;
            }
            default:
                break;
            }
        }
        if (*pos++ != 0x00) {
            throw std::runtime_error("Missing input separator");
        }
    }

    psbt.outputs_.resize(psbt.tx_.outputs().size());
    for (size_t i = 0; i < psbt.outputs_.size() && pos < end; ++i) {
        auto& output = psbt.outputs_[i];
        while (pos < end && *pos != 0x00) {
            auto key_len = decodeVarInt(pos, end);
            if (pos + key_len > end) {
                throw std::runtime_error("Invalid key length");
            }
            uint8_t type = *pos++;
            std::vector<unsigned char> key(pos, pos + key_len - 1);
            pos += key_len - 1;

            auto value_len = decodeVarInt(pos, end);
            if (pos + value_len > end) {
                throw std::runtime_error("Invalid value length");
            }
            std::vector<unsigned char> value(pos, pos + value_len);
            pos += value_len;

            switch (type) {
            case 0x00:
                output.redeem_script = value;
                break;
            case 0x01:
                output.witness_script = value;
                break;
            default:
                break;
            }
        }
        if (pos < end && *pos++ != 0x00) {
            throw std::runtime_error("Missing output separator");
        }
    }

    return psbt;
}

std::vector<unsigned char> PSBT::encodeVarInt(uint64_t n)
{
    std::vector<unsigned char> result;
    if (n < 0xFD) {
        result.push_back(static_cast<unsigned char>(n));
    } else if (n <= 0xFFFF) {
        result.push_back(0xFD);
        result.push_back(n & 0xFF);
        result.push_back((n >> 8) & 0xFF);
    } else if (n <= 0xFFFFFFFF) {
        result.push_back(0xFE);
        for (int i = 0; i < 4; ++i) {
            result.push_back((n >> (i * 8)) & 0xFF);
        }
    } else {
        result.push_back(0xFF);
        for (int i = 0; i < 8; ++i) {
            result.push_back((n >> (i * 8)) & 0xFF);
        }
    }

    return result;
}

uint64_t PSBT::decodeVarInt(const unsigned char*& pos, const unsigned char* end)
{
    if (pos >= end) {
        throw std::runtime_error("Invalid varint");
    }
    uint8_t first = *pos++;
    if (first < 0xFD) {
        return first;
    }
    if (first == 0xFD) {
        if (pos + 2 > end) {
            throw std::runtime_error("Invalid varint");
        }
        uint64_t result = *pos++;
        result |= static_cast<uint64_t>(*pos++) << 8;
        return result;
    }
    if (first == 0xFE) {
        if (pos + 4 > end) {
            throw std::runtime_error("Invalid varint");
        }
        uint64_t result = 0;
        for (int i = 0; i < 4; ++i) {
            result |= static_cast<uint64_t>(*pos++) << (i * 8);
        }
        return result;
    }
    if (pos + 8 > end) {
        throw std::runtime_error("Invalid varint");
    }
    uint64_t result = 0;
    for (int i = 0; i < 8; ++i) {
        result |= static_cast<uint64_t>(*pos++) << (i * 8);
    }

    return result;
}

void PSBT::encodeKeyValue(std::vector<unsigned char>& result, uint8_t type,
    const std::vector<unsigned char>& key,
    const std::vector<unsigned char>& value)
{
    auto key_len = encodeVarInt(key.size() + 1);
    result.insert(result.end(), key_len.begin(), key_len.end());
    result.push_back(type);
    result.insert(result.end(), key.begin(), key.end());

    auto value_len = encodeVarInt(value.size());
    result.insert(result.end(), value_len.begin(), value_len.end());
    result.insert(result.end(), value.begin(), value.end());
}
}
