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
#include <stdexcept>
#include <string>
#include <vector>

namespace bitcoin {

struct TxInput {
    std::array<unsigned char, 32> txid;    // Previous transaction ID
    uint32_t vout;                         // Output index
    std::vector<unsigned char> script_sig; // ScriptSig (for legacy)
    std::vector<unsigned char> witness;    // Witness data (for SegWit/Taproot)
    uint32_t sequence;                     // Sequence number
};

struct TxOutput {
    uint64_t value;                           // Amount in satoshis
    std::vector<unsigned char> script_pubkey; // ScriptPubKey
};

class Transaction {
public:
    Transaction(uint32_t version = 2, uint32_t locktime = 0);

    // Add input
    void addInput(const TxInput& input);

    // Add output
    void addOutput(const TxOutput& output);

    // Serialize transaction to raw bytes
    [[nodiscard]] std::vector<unsigned char> serialize() const;

    // Get transaction ID (double SHA256 of serialized tx)
    [[nodiscard]] std::array<unsigned char, 32> getTxid() const;

    // Get signature hash for signing an input (SIGHASH_ALL default)
    // Supports SegWit (BIP-143) and Taproot (BIP-341) sighash algorithms
    [[nodiscard]] std::array<unsigned char, 32> getSigHash(size_t input_index,
        const std::vector<unsigned char>& script_code, uint64_t value,
        uint8_t sighash_type = 0x01, bool is_taproot = false) const;

    // Access inputs and outputs
    std::vector<TxInput>& inputs()
    {
        return inputs_;
    }

    const std::vector<TxInput>& inputs() const
    {
        return inputs_;
    }

    std::vector<TxOutput>& outputs()
    {
        return outputs_;
    }

    const std::vector<TxOutput>& outputs() const
    {
        return outputs_;
    }

private:
    uint32_t version_;              // Transaction version
    std::vector<TxInput> inputs_;   // List of inputs
    std::vector<TxOutput> outputs_; // List of outputs
    uint32_t locktime_;             // Locktime for transaction
};
}
