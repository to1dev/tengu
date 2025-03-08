#include "SuiWallet.h"

#include "../Utils/Hex.hpp"

namespace Daitengu::Wallets {

struct IntIdentity {
    int operator()(int x) const
    {
        return x;
    }
};

template <int frombits, int tobits, bool pad, typename O, typename It,
    typename I = IntIdentity>
static inline bool ConvertBits(O outfn, It it, It end, I infn = {})
{
    size_t acc = 0;
    size_t bits = 0;
    constexpr size_t maxv = (1 << tobits) - 1;
    constexpr size_t max_acc = (1 << (frombits + tobits - 1)) - 1;
    while (it != end) {
        int v = infn(*it);
        if (v < 0)
            return false;
        acc = ((acc << frombits) | v) & max_acc;
        bits += frombits;
        while (bits >= tobits) {
            bits -= tobits;
            outfn((acc >> bits) & maxv);
        }
        ++it;
    }
    if (pad) {
        if (bits)
            outfn((acc << (tobits - bits)) & maxv);
    } else if (bits >= frombits || ((acc << (tobits - bits)) & maxv)) {
        return false;
    }
    return true;
}

SuiWallet::SuiWallet(Network::Type network)
    : ChainWallet(ChainType::SUI, network)
{
}

void SuiWallet::fromPrivateKey(const std::string& privateKey)
{
    std::memset(&node_, 0, sizeof(HDNode));

    if (privateKey.substr(0, strlen(SUI_PRIVATE_KEY_BECH32_PREFIX))
        == SUI_PRIVATE_KEY_BECH32_PREFIX) {
        size_t max_len = privateKey.size();
        char hrp[84] = { 0 };
        std::vector<uint8_t> data(max_len);
        size_t data_len;

        int result
            = bech32_decode(hrp, data.data(), &data_len, privateKey.c_str());
        if (result != 1) {
            throw std::invalid_argument("Invalid bech32 encoded private key");
        }

        data.resize(data_len);

        if (std::string(hrp) != SUI_PRIVATE_KEY_PREFIX) {
            throw std::invalid_argument("Invalid private key prefix, expected '"
                + std::string(SUI_PRIVATE_KEY_PREFIX) + "'");
        }

        std::vector<uint8_t> bytes;
        bool success = ConvertBits<5, 8, false>(
            [&bytes](uint8_t v) { bytes.push_back(v); }, data.begin(),
            data.end());

        if (!success) {
            throw std::invalid_argument(
                "Invalid private key: conversion failed");
        }

        if (bytes.size() != 33) {
            throw std::invalid_argument("Invalid private key length");
        }

        if (bytes[0] != SCHEME_ED25519) {
            throw std::invalid_argument("Unsupported signature scheme");
        }

        std::memcpy(node_.private_key, bytes.data() + 1, 32);
    } else {
        std::vector<unsigned char> raw;
        if (!HexToBytes(privateKey, raw) || raw.size() != 32) {
            throw std::invalid_argument(
                "Invalid Sui private key, must be 64 hex chars => 32 bytes");
        }

        std::memset(&node_, 0, sizeof(HDNode));
        std::memcpy(node_.private_key, raw.data(), 32);
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

    const uint8_t* privateKeyBytes = node_.private_key;

    std::vector<uint8_t> flaggedPrivateKey(33);
    flaggedPrivateKey[0] = SCHEME_ED25519;
    std::copy(
        privateKeyBytes, privateKeyBytes + 32, flaggedPrivateKey.begin() + 1);

    std::vector<uint8_t> words;
    ConvertBits<8, 5, true>([&words](uint8_t v) { words.push_back(v); },
        flaggedPrivateKey.begin(), flaggedPrivateKey.end());

    char output[128];
    bech32_encode(output, SUI_PRIVATE_KEY_PREFIX, words.data(), words.size(),
        BECH32_ENCODING_BECH32);

    return std::string(output);
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
