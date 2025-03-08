#include "SuiWallet.h"

#include "Utils/Base58.hpp"

#include "../Utils/Hex.hpp"

namespace Daitengu::Wallets {

SuiWallet::SuiWallet(Network::Type network)
    : ChainWallet(ChainType::SUI, network)
{
}

void SuiWallet::fromPrivateKey(const std::string& privateKey)
{
    std::vector<unsigned char> raw;
    if (!HexToBytes(privateKey, raw) || raw.size() != 32) {
        throw std::invalid_argument(
            "Invalid Sui private key, must be 64 hex chars => 32 bytes");
    }

    std::memset(&node_, 0, sizeof(HDNode));

    std::memcpy(node_.private_key, raw.data(), 32);

    std::uint8_t expected_public_key[32];
    ed25519_publickey(node_.private_key, expected_public_key);

    if (std::memcmp(expected_public_key, raw.data() + 32, 32) != 0) {
        throw std::invalid_argument("Invalid private key: public key mismatch");
    }

    seed_.clear();
    mnemonic_.clear();
}

std::string SuiWallet::getAddress(std::uint32_t index)
{
    if (!mnemonic_.empty())
        initNode(index);

    return generateSuiAddress();
}

std::string SuiWallet::getPrivateKey(std::uint32_t index)
{
    if (!mnemonic_.empty())
        initNode(index);

    return std::string();
}

BaseWallet::KeyPair SuiWallet::deriveKeyPair(std::uint32_t index)
{
    return KeyPair {
        getAddress(index),
        getPrivateKey(index),
    };
}

void SuiWallet::onNetworkChanged()
{
}

void SuiWallet::initNode(std::uint32_t index)
{
    if (hdnode_from_seed(seed_.data(), SEED_SIZE, "ed25519", &node_) != 1) {
        throw std::runtime_error("Failed to create HD node from seed.");
    }

    std::uint32_t path[] = {
        44 | HARDENED,    // 44'
        784 | HARDENED,   // 784'
        0 | HARDENED,     // 0'
        0 | HARDENED,     // 0'
        index | HARDENED, // 0'
    };

    if (hdnode_private_ckd_cached(
            &node_, path, sizeof(path) / sizeof(std::uint32_t), nullptr)
        != 1) {
        throw std::runtime_error("Failed to derive key path.");
    }
}

std::string SuiWallet::generateSuiAddress() const
{
    uint8_t public_key[32];
    ed25519_publickey(node_.private_key, public_key);

    std::vector<unsigned char> data;
    data.reserve(33);
    data.push_back(SCHEME_ED25519);
    data.insert(data.end(), public_key, public_key + 32);

    unsigned char hash32[32];
    crypto_generichash_blake2b(
        hash32, 32, data.data(), data.size(), nullptr, 0);

    std::string hexAddr = BytesToHex(hash32, 32);
    for (auto& c : hexAddr) {
        c = std::tolower(static_cast<unsigned char>(c));
    }

    return "0x" + hexAddr;
}

}
