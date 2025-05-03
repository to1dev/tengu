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
#include <iostream>
#include <span>
#include <stdexcept>

#include <sodium.h>

#include "Builder.h"
#include "Types.h"

namespace solana {

namespace {
    const char* ERROR_MESSAGES[] = {
        "Success",                                       // Success
        "Invalid private key data",                      // InvalidPrivateKey
        "Invalid private key length (must be 64 bytes)", // InvalidPrivateKeyLength
        "Invalid pubkey string",                         // InvalidPubkey
        "Invalid blockhash string",                      // InvalidBlockhash
        "Transaction serialization failed",              // SerializationFailed
        "Transaction signing failed",                    // SigningFailed
        "Buffer too small for serialized transaction", // BufferTooSmall
        "Library initialization failed",               // LibraryInitFailed
        "Unknown error"                                // UnknownError
    };
}

const char* SolanaTxBuilder::getErrorMessage(SolanaErrorCode code)
{
    size_t index = static_cast<size_t>(code);
    if (index <= static_cast<size_t>(SolanaErrorCode::LibraryInitFailed)) {
        return ERROR_MESSAGES[index];
    }
    return ERROR_MESSAGES[9];
}

SolanaErrorCode SolanaTxBuilder::buildTransferTransaction(
    const uint8_t* privateKey, const char* toPubkeyStr, uint64_t lamports,
    const char* recentBlockhashStr, uint8_t* txBuffer, size_t bufferSize,
    size_t* actualSize)
{
    try {
        if (sodium_init() < 0) {
            return SolanaErrorCode::LibraryInitFailed;
        }

        if (!privateKey || !toPubkeyStr || !recentBlockhashStr || !txBuffer
            || !actualSize) {
            return SolanaErrorCode::UnknownError;
        }

        std::vector<uint8_t> privateKeyVec(privateKey, privateKey + 64);

        Pubkey fromPubkey;
        try {
            fromPubkey = getPublicKey(privateKeyVec);
        } catch (...) {
            return SolanaErrorCode::InvalidPrivateKey;
        }

        Pubkey toPubkey;
        try {
            toPubkey = pubkeyFromBase58(toPubkeyStr);
        } catch (...) {
            return SolanaErrorCode::InvalidPubkey;
        }

        Blockhash recentBlockhash;
        try {
            recentBlockhash = blockhashFromHexString(recentBlockhashStr);
        } catch (...) {
            return SolanaErrorCode::InvalidBlockhash;
        }

        TransactionBuilder builder;
        builder.setPayer(fromPubkey);
        builder.setRecentBlockhash(recentBlockhash);

        TransactionInstruction transferIx
            = createTransferInstruction(fromPubkey, toPubkey, lamports);
        builder.addInstruction(transferIx);

        Transaction transaction;
        try {
            transaction = builder.build(privateKeyVec);
        } catch (...) {
            return SolanaErrorCode::SigningFailed;
        }

        std::vector<uint8_t> serialized;
        try {
            serialized = transaction.serialize();
        } catch (...) {
            return SolanaErrorCode::SerializationFailed;
        }

        if (serialized.size() > bufferSize) {
            return SolanaErrorCode::BufferTooSmall;
        }
        std::memcpy(txBuffer, serialized.data(), serialized.size());
        *actualSize = serialized.size();

        return SolanaErrorCode::Success;
    } catch (...) {
        return SolanaErrorCode::UnknownError;
    }
}

SolanaErrorCode SolanaTxBuilder::createTokenWithATA(const uint8_t* privateKey,
    uint8_t decimals, uint8_t* txBuffer, size_t bufferSize, size_t* actualSize,
    const char* recentBlockhashStr)
{
    Pubkey payer
        = getPublicKey(std::vector<uint8_t>(privateKey, privateKey + 64));
    Pubkey mint
        = getPublicKey(std::vector<uint8_t>(privateKey + 32, privateKey + 64));
    Pubkey ata = getAssociatedTokenAccount(payer, mint);

    TransactionBuilder builder;
    builder.setPayer(payer);
    builder.setRecentBlockhash(blockhashFromHexString(recentBlockhashStr));
    builder.addInstruction(createMintInstruction(payer, mint, payer, decimals));
    builder.addInstruction(
        createAssociatedTokenAccountInstruction(payer, payer, mint));

    Transaction tx
        = builder.build(std::vector<uint8_t>(privateKey, privateKey + 64));
    std::vector<uint8_t> serialized = tx.serialize();

    if (serialized.size() > bufferSize) {
        return SolanaErrorCode::BufferTooSmall;
    }
    std::memcpy(txBuffer, serialized.data(), serialized.size());
    *actualSize = serialized.size();

    return SolanaErrorCode::Success;
}

SolanaErrorCode SolanaTxBuilder::getPublicKeyFromPrivateKey(
    const uint8_t* privateKey, uint8_t* pubkeyBuffer, size_t* actualSize)
{
    try {
        if (sodium_init() < 0) {
            return SolanaErrorCode::LibraryInitFailed;
        }

        if (!privateKey || !pubkeyBuffer || !actualSize) {
            return SolanaErrorCode::UnknownError;
        }

        if (crypto_sign_ed25519_sk_to_pk(pubkeyBuffer, privateKey) != 0) {
            return SolanaErrorCode::InvalidPrivateKey;
        }

        *actualSize = PUBKEY_LENGTH;
        return SolanaErrorCode::Success;
    } catch (...) {
        return SolanaErrorCode::UnknownError;
    }
}

SolanaErrorCode SolanaTxBuilder::getPublicKeyAsBase58(const uint8_t* privateKey,
    char* pubkeyStrBuffer, size_t bufferSize, size_t* actualSize)
{
    try {
        if (sodium_init() < 0) {
            return SolanaErrorCode::LibraryInitFailed;
        }

        if (!privateKey || !pubkeyStrBuffer || !actualSize) {
            return SolanaErrorCode::UnknownError;
        }

        uint8_t pubkey[PUBKEY_LENGTH];
        size_t pubkeySize = 0;

        SolanaErrorCode result
            = getPublicKeyFromPrivateKey(privateKey, pubkey, &pubkeySize);
        if (result != SolanaErrorCode::Success) {
            return result;
        }

        Pubkey pubkeyObj;
        std::memcpy(pubkeyObj.data(), pubkey, PUBKEY_LENGTH);
        std::string base58Str = pubkeyToBase58(pubkeyObj);

        if (base58Str.length() + 1 > bufferSize) {
            return SolanaErrorCode::BufferTooSmall;
        }

        std::strcpy(pubkeyStrBuffer, base58Str.c_str());
        *actualSize = base58Str.length();

        return SolanaErrorCode::Success;
    } catch (...) {
        return SolanaErrorCode::UnknownError;
    }
}
}
