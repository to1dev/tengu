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

#include <cstring>
#include <iomanip>
#include <sstream>

#include <sodium.h>

#include "Utils/Base58.hpp"

#include "Bincode.hpp"
#include "Borsh.hpp"
#include "Types.h"

namespace solana {

const Pubkey SYSTEM_PROGRAM_ID
    = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

const Pubkey SPL_TOKEN_PROGRAM_ID
    = { 0x06, 0xdd, 0xf6, 0xe1, 0xd7, 0x65, 0xa1, 0x93, 0xd9, 0xcb, 0xe1, 0x46,
          0xce, 0xeb, 0x79, 0xac, 0x1c, 0xb4, 0x85, 0xed, 0x5f, 0x5b, 0x37,
          0x91, 0x3a, 0x8c, 0xf5, 0x85, 0x7e, 0xff, 0x00, 0xa9 };

PDA findProgramAddress(
    const std::vector<std::vector<uint8_t>>& seeds, const Pubkey& programId)
{
    if (sodium_init() < 0) {
        throw std::runtime_error("Failed to initialize libsodium");
    }

    std::vector<uint8_t> buffer;
    size_t total_size = 0;
    for (const auto& seed : seeds) {
        total_size += seed.size();
    }
    total_size += PUBKEY_SIZE + 1;
    buffer.reserve(total_size);

    for (uint8_t bump = 255; bump >= 0; --bump) {
        buffer.clear();

        for (const auto& seed : seeds) {
            buffer.insert(buffer.end(), seed.begin(), seed.end());
        }

        buffer.insert(buffer.end(), programId.begin(), programId.end());

        buffer.push_back(bump);

        Pubkey hash;
        if (crypto_hash_sha256(hash.data(), buffer.data(), buffer.size())
            != 0) {
            throw std::runtime_error("SHA-256 computation failed");
        }

        unsigned char scalar[32];
        std::memcpy(scalar, hash.data(), 32);
        unsigned char point[32];
        if (crypto_scalarmult_ed25519_base(point, scalar) != 0) {
            return { hash, bump };
        }
    }

    throw std::runtime_error("No valid PDA found within bump range");
}

Pubkey getAssociatedTokenAccount(const Pubkey& wallet, const Pubkey& mint)
{
    std::vector<std::vector<uint8_t>> seeds
        = { std::vector<uint8_t>(wallet.begin(), wallet.end()),
              std::vector<uint8_t>(
                  SPL_TOKEN_PROGRAM_ID.begin(), SPL_TOKEN_PROGRAM_ID.end()),
              std::vector<uint8_t>(mint.begin(), mint.end()) };

    PDA pda = findProgramAddress(seeds, SPL_TOKEN_PROGRAM_ID);
    return pda.address;
}

Pubkey pubkeyFromBase58(const std::string& base58Str)
{
    std::vector<unsigned char> decoded;
    if (!DecodeBase58(base58Str, decoded, PUBKEY_SIZE)) {
        throw std::invalid_argument("Invalid base58 string for pubkey");
    }
    if (decoded.size() != PUBKEY_SIZE) {
        throw std::invalid_argument("Decoded pubkey has wrong size");
    }

    Pubkey result;
    std::copy(decoded.begin(), decoded.end(), result.begin());
    return result;
}

std::string pubkeyToBase58(const Pubkey& pubkey)
{
    return EncodeBase58(
        std::span<const unsigned char>(pubkey.data(), pubkey.size()));
}

Blockhash blockhashFromHexString(const std::string& hexString)
{
    if (hexString.size() != 64) {
        throw std::invalid_argument(
            "Blockhash hex string must be 64 characters");
    }

    Blockhash result;
    for (size_t i = 0; i < 32; i++) {
        std::string byteStr = hexString.substr(i * 2, 2);
        result[i] = static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16));
    }
    return result;
}

Pubkey getPublicKey(const std::vector<uint8_t>& privateKey)
{
    if (privateKey.size() != 64) {
        throw std::invalid_argument("Private key must be 64 bytes");
    }

    Pubkey publicKey;
    if (crypto_sign_ed25519_sk_to_pk(publicKey.data(), privateKey.data())
        != 0) {
        throw std::runtime_error("Failed to derive public key");
    }
    return publicKey;
}

static Signature signMessage(
    const std::vector<uint8_t>& message, const std::vector<uint8_t>& privateKey)
{
    if (privateKey.size() != 64) {
        throw std::invalid_argument("Private key must be 64 bytes");
    }

    /*unsigned char fullPrivateKey[crypto_sign_ed25519_SECRETKEYBYTES];
    std::memcpy(fullPrivateKey, privateKey.data(), 32);

    crypto_sign_ed25519_sk_to_pk(fullPrivateKey + 32, fullPrivateKey);*/

    Signature signature;
    unsigned char sig[crypto_sign_ed25519_BYTES];
    unsigned long long sigLen;

    if (crypto_sign_ed25519_detached(
            sig, &sigLen, message.data(), message.size(), privateKey.data())
        != 0) {
        throw std::runtime_error("Signing failed");
    }

    std::memcpy(signature.data(), sig, signature.size());

    // sodium_memzero(fullPrivateKey, sizeof(fullPrivateKey));
    return signature;
}

std::vector<uint8_t> Message::serialize() const
{
    // bincode::BincodeWriter writer;
    borsh::BorshWriter writer;

    writer.write_u8(header.numRequiredSignatures);
    writer.write_u8(header.numReadOnlySignedAccounts);
    writer.write_u8(header.numReadOnlyUnsignedAccounts);

    writer.write_compact_u64(accountKeys.size());
    for (const auto& key : accountKeys) {
        writer.write_fixed_bytes(key);
    }

    writer.write_fixed_bytes(recentBlockhash);

    writer.write_compact_u64(instructions.size());
    for (const auto& ix : instructions) {
        writer.write_u8(ix.programIdIndex);

        writer.write_compact_u64(ix.accountIndexes.size());
        for (const auto& idx : ix.accountIndexes) {
            writer.write_u8(idx);
        }

        writer.write_compact_u64(ix.data.size());
        for (const auto& byte : ix.data) {
            writer.write_u8(byte);
        }
    }

    return writer.get_buffer();
}

std::vector<uint8_t> Transaction::serialize() const
{
    // bincode::BincodeWriter writer;
    borsh::BorshWriter writer;

    writer.write_compact_u64(signatures.size());
    for (const auto& sig : signatures) {
        writer.write_fixed_bytes(sig);
    }

    std::vector<uint8_t> msgData = message.serialize();
    for (auto b : msgData) {
        writer.write_u8(b);
    }

    return writer.get_buffer();
}

void Transaction::sign(
    const std::vector<uint8_t>& privateKey, const Blockhash& recentBlockhash)
{
    message.recentBlockhash = recentBlockhash;

    std::vector<uint8_t> messageData = message.serialize();
    Signature signature = signMessage(messageData, privateKey);

    signatures.clear();
    signatures.push_back(signature);
}

void Transaction::multiSign(
    const std::vector<std::vector<uint8_t>>& privateKeys,
    const Blockhash& recentBlockhash)
{
    // TODO
}

TransactionInstruction createTransferInstruction(
    const Pubkey& fromPubkey, const Pubkey& toPubkey, uint64_t lamports)
{
    // bincode::BincodeWriter writer;
    borsh::BorshWriter writer;

    // writer.write_u64(static_cast<uint64_t>(SystemInstructionType::Transfer));
    writer.write_u32(static_cast<uint32_t>(SystemInstructionType::Transfer));

    writer.write_u64(lamports);

    TransactionInstruction instruction;
    instruction.programId = SYSTEM_PROGRAM_ID;
    instruction.data = writer.get_buffer();

    instruction.accounts.push_back(AccountMeta(fromPubkey, true, true));
    instruction.accounts.push_back(AccountMeta(toPubkey, false, true));

    return instruction;
}

TransactionBuilder::TransactionBuilder()
    : instructions()
    , payer()
    , recentBlockhash()
    , hasSetPayer(false)
    , hasSetRecentBlockhash(false)
{
}

TransactionBuilder& TransactionBuilder::addInstruction(
    const TransactionInstruction& instruction)
{
    instructions.push_back(instruction);
    return *this;
}

TransactionBuilder& TransactionBuilder::setPayer(const Pubkey& payerPubkey)
{
    payer = payerPubkey;
    hasSetPayer = true;
    return *this;
}

TransactionBuilder& TransactionBuilder::setRecentBlockhash(
    const Blockhash& blockhash)
{
    recentBlockhash = blockhash;
    hasSetRecentBlockhash = true;
    return *this;
}

Transaction TransactionBuilder::build(const std::vector<uint8_t>& privateKey)
{
    if (!hasSetPayer) {
        throw std::runtime_error("Payer not set");
    }

    if (!hasSetRecentBlockhash) {
        throw std::runtime_error("Recent blockhash not set");
    }

    if (privateKey.size() != 64) {
        throw std::invalid_argument("Private key must be 64 bytes");
    }

    Pubkey signerPubkey = getPublicKey(privateKey);
    if (signerPubkey != payer) {
        throw std::invalid_argument("Private key does not match payer pubkey");
    }

    Transaction transaction;

    std::map<std::string, AccountMeta> accountMap;
    std::string payerStr = pubkeyToBase58(payer);
    accountMap[payerStr] = AccountMeta(payer, true, true);

    for (const auto& instruction : instructions) {
        std::string programIdStr = pubkeyToBase58(instruction.programId);
        if (accountMap.find(programIdStr) == accountMap.end()) {
            accountMap[programIdStr]
                = AccountMeta(instruction.programId, false, false);
        }
        for (const auto& account : instruction.accounts) {
            std::string accountStr = pubkeyToBase58(account.pubkey);
            if (accountMap.find(accountStr) == accountMap.end()) {
                accountMap[accountStr] = account;
            } else {
                AccountMeta& existing = accountMap[accountStr];
                existing.isSigner = existing.isSigner || account.isSigner;
                existing.isWritable = existing.isWritable || account.isWritable;
            }
        }
    }

    std::vector<AccountMeta> sortedAccounts;
    sortedAccounts.reserve(accountMap.size());

    for (const auto& [key, meta] : accountMap) {
        if (meta.isSigner) {
            sortedAccounts.push_back(meta);
        }
    }

    std::sort(sortedAccounts.begin(), sortedAccounts.end(),
        [](const AccountMeta& a, const AccountMeta& b) {
            return pubkeyToBase58(a.pubkey) < pubkeyToBase58(b.pubkey);
        });

    std::vector<AccountMeta> nonSigners;
    for (const auto& [key, meta] : accountMap) {
        if (!meta.isSigner) {
            nonSigners.push_back(meta);
        }
    }

    std::sort(nonSigners.begin(), nonSigners.end(),
        [](const AccountMeta& a, const AccountMeta& b) {
            return pubkeyToBase58(a.pubkey) < pubkeyToBase58(b.pubkey);
        });

    sortedAccounts.insert(
        sortedAccounts.end(), nonSigners.begin(), nonSigners.end());

    size_t numRequiredSignatures = 0;
    size_t numReadOnlySignedAccounts = 0;
    size_t numReadOnlyUnsignedAccounts = 0;

    for (const auto& meta : sortedAccounts) {
        if (meta.isSigner) {
            numRequiredSignatures++;
            if (!meta.isWritable) {
                numReadOnlySignedAccounts++;
            }
        } else {
            if (!meta.isWritable) {
                numReadOnlyUnsignedAccounts++;
            }
        }
    }

    transaction.message.header.numRequiredSignatures
        = static_cast<uint8_t>(numRequiredSignatures);
    transaction.message.header.numReadOnlySignedAccounts
        = static_cast<uint8_t>(numReadOnlySignedAccounts);
    transaction.message.header.numReadOnlyUnsignedAccounts
        = static_cast<uint8_t>(numReadOnlyUnsignedAccounts);

    for (const auto& meta : sortedAccounts) {
        transaction.message.accountKeys.push_back(meta.pubkey);
    }

    transaction.message.recentBlockhash = recentBlockhash;

    for (const auto& instruction : instructions) {
        CompiledInstruction compiledIx;
        auto programIt = std::find(transaction.message.accountKeys.begin(),
            transaction.message.accountKeys.end(), instruction.programId);
        if (programIt == transaction.message.accountKeys.end()) {
            throw std::runtime_error("Program ID not found in account keys");
        }
        compiledIx.programIdIndex = static_cast<uint8_t>(
            programIt - transaction.message.accountKeys.begin());

        for (const auto& accountMeta : instruction.accounts) {
            auto accountIt = std::find(transaction.message.accountKeys.begin(),
                transaction.message.accountKeys.end(), accountMeta.pubkey);
            if (accountIt == transaction.message.accountKeys.end()) {
                throw std::runtime_error("Account not found in account keys");
            }
            compiledIx.accountIndexes.push_back(static_cast<uint8_t>(
                accountIt - transaction.message.accountKeys.begin()));
        }

        compiledIx.data = instruction.data;
        transaction.message.instructions.push_back(compiledIx);
    }

    transaction.sign(privateKey, recentBlockhash);
    return transaction;
}
}
