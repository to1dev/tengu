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

#include "EthereumWallet.h"

#include "Context.h"

namespace Daitengu::Wallets {

EthereumWallet::EthereumWallet(Network::Type network)
    : ChainWallet(ChainType::ETHEREUM, network)
{
}

void EthereumWallet::fromPrivateKey(const std::string& privateKey)
{
    std::memset(&node_, 0, sizeof(HDNode));
    seed_.clear();
    mnemonic_.clear();

    std::vector<unsigned char> decoded;
    if (!HexToBytes(privateKey, decoded) || decoded.size() != 32) {
        throw std::invalid_argument(
            "Invalid hex private key (must be 32 bytes)");
    }

    std::memcpy(node_.private_key, decoded.data(), 32);

    node_.curve = get_curve_by_name("secp256k1");
    if (!node_.curve) {
        throw std::runtime_error("get_curve_by_name(\"secp256k1\") returned "
                                 "null; check your trezor-crypto build");
    }

    fillUncompressedPublicKey();
}

std::string EthereumWallet::getAddress(std::uint32_t index)
{
    if (!mnemonic_.empty()) {
        initNode(index);
    }

    return generateEthereumAddress();
}

std::string EthereumWallet::getPrivateKey(std::uint32_t index)
{
    if (!mnemonic_.empty()) {
        initNode(index);
    }

    return BytesToHex(node_.private_key, 32);
}

BaseWallet::KeyPair EthereumWallet::deriveKeyPair(std::uint32_t index)
{
    return KeyPair {
        getAddress(index),
        getPrivateKey(index),
    };
}

void EthereumWallet::onNetworkChanged()
{
}

void EthereumWallet::initNode(std::uint32_t index)
{
    if (hdnode_from_seed(seed_.data(), seed_.size(), "secp256k1", &node_)
        != 1) {
        throw std::runtime_error("Failed to create HD node from seed");
    }

    std::uint32_t path[] = {
        44 | HARDENED, // 44'
        60 | HARDENED, // 60'
        0 | HARDENED,  // 0'
        0,             // change=0
        index,
    };

    if (hdnode_private_ckd_cached(
            &node_, path, sizeof(path) / sizeof(std::uint32_t), nullptr)
        != 1) {
        throw std::runtime_error("Failed to derive path for EthereumWallet");
    }

    node_.curve = get_curve_by_name("secp256k1");
    if (!node_.curve) {
        throw std::runtime_error("Missing secp256k1 curve info");
    }

    fillUncompressedPublicKey();
}

std::string EthereumWallet::generateEthereumAddress() const
{
    if (uncompressedPub_[0] != 0x04) {
        throw std::runtime_error(
            "Public key is not uncompressed! uncompressedPub_[0] != 0x04");
    }

    unsigned char hash32[32];
    keccak_256(uncompressedPub_ + 1, 64, hash32);

    std::array<unsigned char, 20> addrBytes;
    std::memcpy(addrBytes.data(), hash32 + 12, 20);

    std::string hexStr = BytesToHex(addrBytes.data(), 20);
    for (auto& c : hexStr) {
        c = std::tolower(static_cast<unsigned char>(c));
    }
    return "0x" + hexStr;
}

void EthereumWallet::fillUncompressedPublicKey()
{
    int ret = ecdsa_get_public_key65(
        node_.curve->params, node_.private_key, uncompressedPub_);
    if (ret != 0) {
        throw std::runtime_error("ecdsa_get_public_key65 failed");
    }
}

}
