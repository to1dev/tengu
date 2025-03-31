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

#include <algorithm>
#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace solana {

constexpr size_t PUBKEY_SIZE = 32;
constexpr size_t SIGNATURE_SIZE = 64;
constexpr size_t PRIVATE_KEY_SIZE = 64;

using Pubkey = std::array<uint8_t, PUBKEY_SIZE>;
using Signature = std::array<uint8_t, SIGNATURE_SIZE>;
using Blockhash = std::array<uint8_t, 32>;

struct MessageHeader {
    uint8_t numRequiredSignatures;
    uint8_t numReadOnlySignedAccounts;
    uint8_t numReadOnlyUnsignedAccounts;
};

struct CompiledInstruction {
    uint8_t programIdIndex;
    std::vector<uint8_t> accountIndexes;
    std::vector<uint8_t> data;
};

struct Message {
    MessageHeader header;
    std::vector<Pubkey> accountKeys;
    Blockhash recentBlockhash;
    std::vector<CompiledInstruction> instructions;

    std::vector<uint8_t> serialize() const;
};

struct Transaction {
    std::vector<Signature> signatures;
    Message message;

    std::vector<uint8_t> serialize() const;

    void sign(const std::vector<uint8_t>& privateKey,
        const Blockhash& recentBlockhash);
    void multiSign(const std::vector<std::vector<uint8_t>>& privateKeys,
        const Blockhash& recentBlockhash);
};

struct AccountMeta {
    Pubkey pubkey;
    bool isSigner;
    bool isWritable;

    AccountMeta()
        : pubkey()
        , isSigner(false)
        , isWritable(false)
    {
    }

    AccountMeta(const Pubkey& pk, bool signer, bool writable)
        : pubkey(pk)
        , isSigner(signer)
        , isWritable(writable)
    {
    }
};

struct TransactionInstruction {
    Pubkey programId;
    std::vector<AccountMeta> accounts;
    std::vector<uint8_t> data;
};

enum class SystemInstructionType : uint32_t {
    CreateAccount = 0,
    Assign,
    Transfer,
    CreateAccountWithSeed,
    AdvanceNonceAccount,
    WithdrawNonceAccount,
    InitializeNonceAccount,
    AuthorizeNonceAccount,
    Allocate,
    AllocateWithSeed,
    AssignWithSeed,
    TransferWithSeed
};

Pubkey pubkeyFromBase58(const std::string& base58Str);
std::string pubkeyToBase58(const Pubkey& pubkey);
Pubkey getPublicKey(const std::vector<uint8_t>& privateKey);

TransactionInstruction createTransferInstruction(
    const Pubkey& fromPubkey, const Pubkey& toPubkey, uint64_t lamports);

class TransactionBuilder {
public:
    TransactionBuilder();

    TransactionBuilder& addInstruction(
        const TransactionInstruction& instruction);

    TransactionBuilder& setPayer(const Pubkey& payerPubkey);

    TransactionBuilder& setRecentBlockhash(const Blockhash& blockhash);

    Transaction build(const std::vector<uint8_t>& privateKey);

private:
    std::vector<TransactionInstruction> instructions;
    Pubkey payer;
    Blockhash recentBlockhash;
    bool hasSetPayer;
    bool hasSetRecentBlockhash;
};

Blockhash blockhashFromHexString(const std::string& hexString);
}
