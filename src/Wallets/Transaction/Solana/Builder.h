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

#include <cstddef>
#include <cstdint>
#include <string>

namespace solana {

enum class SolanaErrorCode {
    Success = 0,
    InvalidPrivateKey = 1,
    InvalidPrivateKeyLength = 2,
    InvalidPubkey = 3,
    InvalidBlockhash = 4,
    SerializationFailed = 5,
    SigningFailed = 6,
    BufferTooSmall = 7,
    LibraryInitFailed = 8,
    UnknownError = 99
};

constexpr size_t SOLANA_TX_BUFFER_SIZE = 8 * 1024;

class SolanaTxBuilder {
public:
    static constexpr size_t PUBKEY_LENGTH = 32;
    static constexpr size_t PRIVATE_KEY_LENGTH = 64;

    static SolanaErrorCode buildTransferTransaction(const uint8_t* privateKey,
        const char* toPubkeyStr, uint64_t lamports,
        const char* recentBlockhashStr, uint8_t* txBuffer, size_t bufferSize,
        size_t* actualSize);

    static SolanaErrorCode createTokenWithATA(const uint8_t* privateKey,
        uint8_t decimals, uint8_t* txBuffer, size_t bufferSize,
        size_t* actualSize, const char* recentBlockhashStr);

    static SolanaErrorCode getPublicKeyFromPrivateKey(
        const uint8_t* privateKey, uint8_t* pubkeyBuffer, size_t* actualSize);

    static SolanaErrorCode getPublicKeyAsBase58(const uint8_t* privateKey,
        char* pubkeyStrBuffer, size_t bufferSize, size_t* actualSize);

    static const char* getErrorMessage(SolanaErrorCode code);
};
}
