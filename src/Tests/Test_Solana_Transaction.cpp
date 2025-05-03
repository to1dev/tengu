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

#include <array>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <sodium.h>

#include "Wallets/Transaction/Solana/Builder.h"
#include "Wallets/Transaction/Solana/Types.h"
#include "Wallets/Utils/Hex.hpp"

using namespace solana;

#include "Utils/DotEnv.hpp"
#include "Utils/PathUtils.hpp"

using namespace Daitengu::Utils;

std::string base64Encode(const uint8_t* data, size_t length)
{
    size_t b64_len
        = sodium_base64_encoded_len(length, sodium_base64_VARIANT_ORIGINAL);
    std::string encoded(b64_len, '\0');

    sodium_bin2base64(&encoded[0], encoded.size(), data, length,
        sodium_base64_VARIANT_ORIGINAL);

    return encoded;
}

int main()
{
    if (sodium_init() < 0) {
        std::cerr << "Failed to init libsodium" << std::endl;
        return 1;
    }

    auto currentPath = PathUtils::getExecutableDir();
    auto& parser = DotEnv::getInstance();
    parser.load((currentPath / ".env").string());

    Pubkey wallet = pubkeyFromBase58(*parser.get("ATA_WALLET"));
    Pubkey mint = pubkeyFromBase58(*parser.get("ATA_MINT"));

    Pubkey ata = getAssociatedTokenAccount(wallet, mint);

    std::string ataBase58 = pubkeyToBase58(ata);

    std::cout << "Associated Token Account: " << ataBase58 << std::endl;

    std::cout << "------\n";

    unsigned char seed[crypto_sign_ed25519_SEEDBYTES];
    unsigned char publicKey[crypto_sign_ed25519_PUBLICKEYBYTES];
    unsigned char secretKey[crypto_sign_ed25519_SECRETKEYBYTES];

    randombytes_buf(seed, sizeof(seed));
    crypto_sign_ed25519_seed_keypair(publicKey, secretKey, seed);

    std::array<uint8_t, 64> privateKey;
    std::memcpy(privateKey.data(), secretKey, privateKey.size());

    sodium_memzero(secretKey, sizeof(secretKey));
    sodium_memzero(seed, sizeof(seed));

    uint8_t pubkeyData[32];
    size_t pubkeySize = 0;

    SolanaErrorCode result = SolanaTxBuilder::getPublicKeyFromPrivateKey(
        privateKey.data(), pubkeyData, &pubkeySize);

    if (result != SolanaErrorCode::Success) {
        std::cerr << "Failed to get public key: "
                  << SolanaTxBuilder::getErrorMessage(result) << std::endl;
        return 1;
    }

    std::cout << "Public key (hex): " << BytesToHex(pubkeyData, pubkeySize)
              << std::endl;

    char pubkeyBase58[50];
    size_t base58Size = 0;

    result = SolanaTxBuilder::getPublicKeyAsBase58(
        privateKey.data(), pubkeyBase58, sizeof(pubkeyBase58), &base58Size);

    if (result != SolanaErrorCode::Success) {
        std::cerr << "Failed to get base58 public key: "
                  << SolanaTxBuilder::getErrorMessage(result) << std::endl;
        return 1;
    }

    std::cout << "Public key (base58): " << pubkeyBase58 << std::endl;

    std::cout << "\nBuilding transfer transaction:" << std::endl;

    const char* recipient = "675kPX9MHTjS2zt1qfr1NYHuzeLXfQM9H24wFSUt1Mp8";
    uint64_t amount = 10000000;
    const char* blockhash
        = "931e8e74b3c8d914bd7d4b0f62bf36f7080a467b5bf7e9d2ee4ded19596f0dbd";

    uint8_t txBuffer[SOLANA_TX_BUFFER_SIZE];
    size_t txSize = 0;

    result = SolanaTxBuilder::buildTransferTransaction(privateKey.data(),
        recipient, amount, blockhash, txBuffer, sizeof(txBuffer), &txSize);

    if (result != SolanaErrorCode::Success) {
        std::cerr << "Failed to build transaction: "
                  << SolanaTxBuilder::getErrorMessage(result) << std::endl;
        return 1;
    }

    std::cout << "Transaction built successfully!" << std::endl;
    std::cout << "Transaction size: " << txSize << " bytes" << std::endl;

    std::string txBase64 = base64Encode(txBuffer, txSize);
    std::cout << "Base64 of transaction:\n" << txBase64 << std::endl;

    std::cout << "\nTransaction is ready to be sent to the Solana network!"
              << std::endl;

    sodium_memzero(privateKey.data(), privateKey.size());

    return 0;
}
