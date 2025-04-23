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

#include "Transaction.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "izanagi/sha2.h"

#ifdef __cplusplus
}
#endif

#include "../../Utils/Hex.hpp"

namespace bitcoin {

Transaction::Transaction(uint32_t version, uint32_t locktime)
    : version_(version)
    , locktime_(locktime)
{
}

void Transaction::addInput(const TxInput& input)
{
    inputs_.push_back(input);
}

void Transaction::addOutput(const TxOutput& output)
{
    outputs_.push_back(output);
}

std::vector<unsigned char> Transaction::serialize() const
{
    std::vector<unsigned char> result;

    // Version (4 bytes, little-endian)
    result.push_back(version_ & 0xFF);
    result.push_back((version_ >> 8) & 0xFF);
    result.push_back((version_ >> 16) & 0xFF);
    result.push_back((version_ >> 24) & 0xFF);

    // Input count (varint)
    size_t input_count = inputs_.size();
    if (input_count < 0xFD) {
        result.push_back(static_cast<unsigned char>(input_count));
    } else {
        throw std::runtime_error("Too many inputs");
    }

    // Inputs
    for (const auto& input : inputs_) {
        // Txid (32 bytes, little-endian)
        result.insert(result.end(), input.txid.begin(), input.txid.end());
        // Vout (4 bytes, little-endian)
        result.push_back(input.vout & 0xFF);
        result.push_back((input.vout >> 8) & 0xFF);
        result.push_back((input.vout >> 16) & 0xFF);
        result.push_back((input.vout >> 24) & 0xFF);
        // ScriptSig (varint + data)
        size_t script_sig_len = input.script_sig.size();
        if (script_sig_len < 0xFD) {
            result.push_back(static_cast<unsigned char>(script_sig_len));
        } else {
            throw std::runtime_error("ScriptSig too large");
        }
        result.insert(
            result.end(), input.script_sig.begin(), input.script_sig.end());
        // Sequence (4 bytes, little-endian)
        result.push_back(input.sequence & 0xFF);
        result.push_back((input.sequence >> 8) & 0xFF);
        result.push_back((input.sequence >> 16) & 0xFF);
        result.push_back((input.sequence >> 24) & 0xFF);
    }

    // Output count (varint)
    size_t output_count = outputs_.size();
    if (output_count < 0xFD) {
        result.push_back(static_cast<unsigned char>(output_count));
    } else {
        throw std::runtime_error("Too many outputs");
    }

    // Outputs
    for (const auto& output : outputs_) {
        // Value (8 bytes, little-endian)
        uint64_t value = output.value;
        for (int i = 0; i < 8; ++i) {
            result.push_back((value >> (i * 8)) & 0xFF);
        }
        // ScriptPubKey (varint + data)
        size_t script_len = output.script_pubkey.size();
        if (script_len < 0xFD) {
            result.push_back(static_cast<unsigned char>(script_len));
        } else {
            throw std::runtime_error("ScriptPubKey too large");
        }
        result.insert(result.end(), output.script_pubkey.begin(),
            output.script_pubkey.end());
    }

    // Locktime (4 bytes, little-endian)
    result.push_back(locktime_ & 0xFF);
    result.push_back((locktime_ >> 8) & 0xFF);
    result.push_back((locktime_ >> 16) & 0xFF);
    result.push_back((locktime_ >> 24) & 0xFF);

    return result;
}

std::array<unsigned char, 32> Transaction::getTxid() const
{
    auto serialized = serialize();
    std::array<unsigned char, 32> hash1, hash2;
    sha256_Raw(serialized.data(), serialized.size(), hash1.data());
    sha256_Raw(hash1.data(), 32, hash2.data());

    return hash2;
}

std::array<unsigned char, 32> Transaction::getSigHash(size_t input_index,
    const std::vector<unsigned char>& script_code, uint64_t value,
    uint8_t sighash_type, bool is_taproot) const
{
    if (input_index >= inputs_.size()) {
        throw std::invalid_argument("Invalid input index");
    }

    std::vector<unsigned char> result;
    uint8_t base_sighash = sighash_type & 0x03; // ALL, NONE, SINGLE
    bool anyonecanpay = (sighash_type & 0x80) != 0;

    if (is_taproot) {
        // Taproot sighash (BIP-341)
        result.push_back(0x00);         // Epoch
        result.push_back(sighash_type); // Hash type

        // Version (4 bytes, little-endian)
        result.push_back(version_ & 0xFF);
        result.push_back((version_ >> 8) & 0xFF);
        result.push_back((version_ >> 16) & 0xFF);
        result.push_back((version_ >> 24) & 0xFF);

        // Locktime (4 bytes, little-endian)
        result.push_back(locktime_ & 0xFF);
        result.push_back((locktime_ >> 8) & 0xFF);
        result.push_back((locktime_ >> 16) & 0xFF);
        result.push_back((locktime_ >> 24) & 0xFF);

        // Prevouts hash
        std::vector<unsigned char> prevouts;
        if (!anyonecanpay) {
            for (const auto& input : inputs_) {
                prevouts.insert(
                    prevouts.end(), input.txid.begin(), input.txid.end());
                for (int i = 0; i < 4; ++i) {
                    prevouts.push_back((input.vout >> (i * 8)) & 0xFF);
                }
            }
        } else {
            const auto& input = inputs_[input_index];
            prevouts.insert(
                prevouts.end(), input.txid.begin(), input.txid.end());
            for (int i = 0; i < 4; ++i) {
                prevouts.push_back((input.vout >> (i * 8)) & 0xFF);
            }
        }
        std::array<unsigned char, 32> prevouts_hash;
        sha256_Raw(prevouts.data(), prevouts.size(), prevouts_hash.data());
        result.insert(result.end(), prevouts_hash.begin(), prevouts_hash.end());

        // Amounts hash
        std::vector<unsigned char> amounts;
        if (!anyonecanpay) {
            for (size_t i = 0; i < inputs_.size(); ++i) {
                for (int j = 0; j < 8; ++j) {
                    amounts.push_back((value >> (j * 8)) & 0xFF);
                }
            }
        } else {
            for (int j = 0; j < 8; ++j) {
                amounts.push_back((value >> (j * 8)) & 0xFF);
            }
        }
        std::array<unsigned char, 32> amounts_hash;
        sha256_Raw(amounts.data(), amounts.size(), amounts_hash.data());
        result.insert(result.end(), amounts_hash.begin(), amounts_hash.end());

        // ScriptPubKeys hash
        std::vector<unsigned char> scriptpubkeys;
        if (!anyonecanpay) {
            for (size_t i = 0; i < inputs_.size(); ++i) {
                scriptpubkeys.push_back(
                    static_cast<unsigned char>(script_code.size()));
                scriptpubkeys.insert(scriptpubkeys.end(), script_code.begin(),
                    script_code.end());
            }
        } else {
            scriptpubkeys.push_back(
                static_cast<unsigned char>(script_code.size()));
            scriptpubkeys.insert(
                scriptpubkeys.end(), script_code.begin(), script_code.end());
        }
        std::array<unsigned char, 32> scriptpubkeys_hash;
        sha256_Raw(scriptpubkeys.data(), scriptpubkeys.size(),
            scriptpubkeys_hash.data());
        result.insert(
            result.end(), scriptpubkeys_hash.begin(), scriptpubkeys_hash.end());

        // Sequences hash
        std::vector<unsigned char> sequences;
        if (!anyonecanpay) {
            for (const auto& input : inputs_) {
                for (int i = 0; i < 4; ++i) {
                    sequences.push_back((input.sequence >> (i * 8)) & 0xFF);
                }
            }
        } else {
            const auto& input = inputs_[input_index];
            for (int i = 0; i < 4; ++i) {
                sequences.push_back((input.sequence >> (i * 8)) & 0xFF);
            }
        }
        std::array<unsigned char, 32> sequences_hash;
        sha256_Raw(sequences.data(), sequences.size(), sequences_hash.data());
        result.insert(
            result.end(), sequences_hash.begin(), sequences_hash.end());

        // Outputs hash
        std::vector<unsigned char> outputs;
        if (base_sighash == 0x01) { // SIGHASH_ALL
            for (const auto& output : outputs_) {
                for (int i = 0; i < 8; ++i) {
                    outputs.push_back((output.value >> (i * 8)) & 0xFF);
                }
                outputs.push_back(
                    static_cast<unsigned char>(output.script_pubkey.size()));
                outputs.insert(outputs.end(), output.script_pubkey.begin(),
                    output.script_pubkey.end());
            }
        } else if (base_sighash == 0x03
            && input_index < outputs_.size()) { // SIGHASH_SINGLE
            const auto& output = outputs_[input_index];
            for (int i = 0; i < 8; ++i) {
                outputs.push_back((output.value >> (i * 8)) & 0xFF);
            }
            outputs.push_back(
                static_cast<unsigned char>(output.script_pubkey.size()));
            outputs.insert(outputs.end(), output.script_pubkey.begin(),
                output.script_pubkey.end());
        }
        std::array<unsigned char, 32> outputs_hash;
        sha256_Raw(outputs.data(), outputs.size(), outputs_hash.data());
        result.insert(result.end(), outputs_hash.begin(), outputs_hash.end());

        // Input index (4 bytes, little-endian)
        result.push_back(input_index & 0xFF);
        result.push_back((input_index >> 8) & 0xFF);
        result.push_back((input_index >> 16) & 0xFF);
        result.push_back((input_index >> 24) & 0xFF);
    } else {
        // SegWit sighash (BIP-143)
        result.push_back(version_ & 0xFF);
        result.push_back((version_ >> 8) & 0xFF);
        result.push_back((version_ >> 16) & 0xFF);
        result.push_back((version_ >> 24) & 0xFF);

        // Prevouts hash
        std::vector<unsigned char> prevouts;
        if (!anyonecanpay) {
            for (const auto& input : inputs_) {
                prevouts.insert(
                    prevouts.end(), input.txid.begin(), input.txid.end());
                for (int i = 0; i < 4; ++i) {
                    prevouts.push_back((input.vout >> (i * 8)) & 0xFF);
                }
            }
        } else {
            const auto& input = inputs_[input_index];
            prevouts.insert(
                prevouts.end(), input.txid.begin(), input.txid.end());
            for (int i = 0; i < 4; ++i) {
                prevouts.push_back((input.vout >> (i * 8)) & 0xFF);
            }
        }
        std::array<unsigned char, 32> prevouts_hash;
        sha256_Raw(prevouts.data(), prevouts.size(), prevouts_hash.data());
        result.insert(result.end(), prevouts_hash.begin(), prevouts_hash.end());

        // Sequences hash
        std::vector<unsigned char> sequences;
        if (!anyonecanpay && base_sighash != 0x02) { // Not SIGHASH_NONE
            for (const auto& input : inputs_) {
                for (int i = 0; i < 4; ++i) {
                    sequences.push_back((input.sequence >> (i * 8)) & 0xFF);
                }
            }
        } else {
            const auto& input = inputs_[input_index];
            for (int i = 0; i < 4; ++i) {
                sequences.push_back((input.sequence >> (i * 8)) & 0xFF);
            }
        }
        std::array<unsigned char, 32> sequences_hash;
        sha256_Raw(sequences.data(), sequences.size(), sequences_hash.data());
        result.insert(
            result.end(), sequences_hash.begin(), sequences_hash.end());

        // Current input outpoint
        const auto& input = inputs_[input_index];
        result.insert(result.end(), input.txid.begin(), input.txid.end());
        for (int i = 0; i < 4; ++i) {
            result.push_back((input.vout >> (i * 8)) & 0xFF);
        }

        // ScriptCode
        result.push_back(static_cast<unsigned char>(script_code.size()));
        result.insert(result.end(), script_code.begin(), script_code.end());

        // Value (8 bytes, little-endian)
        for (int i = 0; i < 8; ++i) {
            result.push_back((value >> (i * 8)) & 0xFF);
        }

        // Sequence
        for (int i = 0; i < 4; ++i) {
            result.push_back((input.sequence >> (i * 8)) & 0xFF);
        }

        // Outputs hash
        std::vector<unsigned char> outputs;
        if (base_sighash == 0x01) { // SIGHASH_ALL
            for (const auto& output : outputs_) {
                for (int i = 0; i < 8; ++i) {
                    outputs.push_back((output.value >> (i * 8)) & 0xFF);
                }
                outputs.push_back(
                    static_cast<unsigned char>(output.script_pubkey.size()));
                outputs.insert(outputs.end(), output.script_pubkey.begin(),
                    output.script_pubkey.end());
            }
        } else if (base_sighash == 0x03
            && input_index < outputs_.size()) { // SIGHASH_SINGLE
            const auto& output = outputs_[input_index];
            for (int i = 0; i < 8; ++i) {
                outputs.push_back((output.value >> (i * 8)) & 0xFF);
            }
            outputs.push_back(
                static_cast<unsigned char>(output.script_pubkey.size()));
            outputs.insert(outputs.end(), output.script_pubkey.begin(),
                output.script_pubkey.end());
        }
        std::array<unsigned char, 32> outputs_hash;
        sha256_Raw(outputs.data(), outputs.size(), outputs_hash.data());
        result.insert(result.end(), outputs_hash.begin(), outputs_hash.end());

        // Locktime (4 bytes, little-endian)
        result.push_back(locktime_ & 0xFF);
        result.push_back((locktime_ >> 8) & 0xFF);
        result.push_back((locktime_ >> 16) & 0xFF);
        result.push_back((locktime_ >> 24) & 0xFF);

        // Sighash type (4 bytes, little-endian)
        result.push_back(sighash_type);
        result.push_back(0);
        result.push_back(0);
        result.push_back(0);
    }

    // Compute double SHA256 of the result
    std::array<unsigned char, 32> sighash;
    sha256_Raw(result.data(), result.size(), sighash.data());
    sha256_Raw(sighash.data(), 32, sighash.data());

    return sighash;
}
}
