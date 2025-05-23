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

inline std::string keccak256(std::string_view input)
{
    std::vector<unsigned char> buf(input.begin(), input.end());

    unsigned char digest[32] = { 0 };
    keccak_256(buf.data(), buf.size(), digest);

    static const char* hexChars = "0123456789abcdef";
    std::string hexDigest;
    hexDigest.reserve(64);

    for (int i = 0; i < 32; ++i) {
        unsigned char c = digest[i];
        hexDigest.push_back(hexChars[(c >> 4) & 0xF]);
        hexDigest.push_back(hexChars[c & 0xF]);
    }

    return hexDigest;
}

EthereumWallet::EthereumWallet(bool useEip55, Network::Type network)
    : ChainWallet(ChainType::ETHEREUM, network)
    , useEip55_(useEip55)
{
}

bool EthereumWallet::isValid(std::string_view address)
{
    if (address.size() >= 2 && address[0] == '0'
        && (address[1] == 'x' || address[1] == 'X')) {
        address.remove_prefix(2);
    }

    if (address.size() != 40) {
        return false;
    }

    for (char c : address) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }

    bool isAllLower = true, isAllUpper = true;
    for (char c : address) {
        if (std::isalpha(static_cast<unsigned char>(c))) {
            if (std::islower(static_cast<unsigned char>(c))) {
                isAllUpper = false;
            } else if (std::isupper(static_cast<unsigned char>(c))) {
                isAllLower = false;
            }
        }
    }
    if (isAllLower || isAllUpper) {
        return true;
    }

    std::string lowerAddress(address);
    std::transform(lowerAddress.begin(), lowerAddress.end(),
        lowerAddress.begin(), ::tolower);

    std::string hash = keccak256(lowerAddress);
    if (hash.size() != 64) {
        return false;
    }

    for (size_t i = 0; i < 40; ++i) {
        char addrChar = address[i];
        if (std::isalpha(static_cast<unsigned char>(addrChar))) {
            char hashChar = hash[i];
            int hashValue = 0;
            if (hashChar >= '0' && hashChar <= '9') {
                hashValue = hashChar - '0';
            } else {
                hashValue = std::tolower(hashChar) - 'a' + 10;
            }

            if ((hashValue >= 8
                    && !std::isupper(static_cast<unsigned char>(addrChar)))
                || (hashValue < 8
                    && !std::islower(static_cast<unsigned char>(addrChar)))) {
                return false;
            }
        }
    }

    return true;
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

    std::string eip55Core = hexStr;
    if (useEip55_) {
        eip55Core = toEip55(hexStr);
    }

    return "0x" + eip55Core;
}

void EthereumWallet::fillUncompressedPublicKey()
{
    int ret = ecdsa_get_public_key65(
        node_.curve->params, node_.private_key, uncompressedPub_);
    if (ret != 0) {
        throw std::runtime_error("ecdsa_get_public_key65 failed");
    }
}

std::string EthereumWallet::toEip55(const std::string& addressLower) const
{
    std::vector<unsigned char> lowerBytes(addressLower.size());
    for (size_t i = 0; i < addressLower.size(); i++) {
        lowerBytes[i] = static_cast<unsigned char>(addressLower[i]);
    }
    unsigned char hash32[32];
    keccak_256(lowerBytes.data(), lowerBytes.size(), hash32);

    std::string result;
    result.reserve(40);

    for (size_t i = 0; i < addressLower.size(); i++) {
        char c = addressLower[i];
        if (std::isdigit(static_cast<unsigned char>(c))) {
            result.push_back(c);
        } else {
            int nibbleIndex = i / 2;
            int nibble;
            if ((i % 2) == 0) {
                nibble = (hash32[nibbleIndex] >> 4) & 0x0f;
            } else {
                nibble = hash32[nibbleIndex] & 0x0f;
            }

            if (nibble >= 8) {
                result.push_back(std::toupper(static_cast<unsigned char>(c)));
            } else {
                result.push_back(std::tolower(static_cast<unsigned char>(c)));
            }
        }
    }

    return result;
}

}
