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

#include "SolanaWallet.h"

namespace Daitengu::Wallets {

inline bool isValidBase58(std::string_view text)
{
    static const std::regex base58Regex { R"(^[1-9A-HJ-NP-Za-km-z]{32,44}$)" };
    return std::regex_match(text.begin(), text.end(), base58Regex);
}

SolanaWallet::SolanaWallet(Network::Type network)
    : ChainWallet(ChainType::SOLANA, network)
{
}

bool SolanaWallet::isValid(std::string_view address)
{
    if (!isValidBase58(address)) {
        return false;
    }

    try {
        std::vector<unsigned char> decoded;
        if (!DecodeBase58(std::string(address), decoded, address.size())) {
            return false;
        }

        return decoded.size() == 32;
    } catch (const std::exception& e) {
        std::cerr << "Failed to check Solana address: " << e.what()
                  << std::endl;
    }

    return false;
}

bool SolanaWallet::solanaMode() const
{
    return solanaMode_;
}

void SolanaWallet::setSolanaMode(bool newSolanaMode)
{
    solanaMode_ = newSolanaMode;
}

void SolanaWallet::fromPrivateKey(const std::string& privateKey)
{
    std::vector<unsigned char> decoded;
    if (!DecodeBase58(privateKey, decoded, 64) || decoded.size() != 64) {
        throw std::invalid_argument("Invalid private key format");
    }

    std::memset(&node_, 0, sizeof(HDNode));

    std::memcpy(node_.private_key, decoded.data(), 32);

    std::uint8_t expected_public_key[32];
    ed25519_publickey(node_.private_key, expected_public_key);

    if (std::memcmp(expected_public_key, decoded.data() + 32, 32) != 0) {
        throw std::invalid_argument("Invalid private key: public key mismatch");
    }

    seed_.clear();
    mnemonic_.clear();
}

std::string SolanaWallet::getAddress(uint32_t index)
{
    if (!mnemonic_.empty())
        initNode(index);

    uint8_t public_key[32];
    if (solanaMode_) {
        ed25519_publickey(seed_.data(), public_key);
    } else {
        ed25519_publickey(node_.private_key, public_key);
    }

    std::span<const unsigned char> pubkey_span(public_key, 32);
    return EncodeBase58(pubkey_span);
}

std::string SolanaWallet::getPrivateKey(uint32_t index)
{
    if (!mnemonic_.empty())
        initNode(index);

    std::vector<std::uint8_t> full_private_key(64);
    std::uint8_t public_key[32];

    if (solanaMode_) {
        std::copy(seed_.data(), seed_.data() + 32, full_private_key.begin());
        ed25519_publickey(seed_.data(), public_key);
    } else {
        std::copy(node_.private_key, node_.private_key + 32,
            full_private_key.begin());
        ed25519_publickey(node_.private_key, public_key);
    }

    std::copy(public_key, public_key + 32, full_private_key.begin() + 32);

    return EncodeBase58(std::span<const unsigned char>(full_private_key));
}

void SolanaWallet::onNetworkChanged()
{
}

BaseWallet::KeyPair SolanaWallet::deriveKeyPair(uint32_t index)
{
    auto priv = getPrivateKey(index);
    auto addr = getAddress(index);
    return KeyPair {
        addr,
        priv,
    };
}

void SolanaWallet::initNode(uint32_t index)
{
    if (hdnode_from_seed(seed_.data(), SEED_SIZE, "ed25519", &node_) != 1) {
        throw std::runtime_error("Failed to create HD node from seed.");
    }

    if (!solanaMode_) {
        std::uint32_t path[] = {
            44 | HARDENED,    // 44'
            501 | HARDENED,   // 501'
            index | HARDENED, // 0'
            0 | HARDENED      // 0'
        };

        if (hdnode_private_ckd_cached(
                &node_, path, sizeof(path) / sizeof(std::uint32_t), nullptr)
            != 1) {
            throw std::runtime_error("Failed to derive key path.");
        }
    }
}

}
