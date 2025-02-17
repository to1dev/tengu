#include "SolanaWallet.h"

namespace Daitengu::Wallets {

SolanaWallet::SolanaWallet(bool solanaMode)
    : solanaMode_(solanaMode)
{
}

SolanaWallet::~SolanaWallet()
{
    cleanup();
}

void SolanaWallet::fromPrivateKey(const std::string& privateKey)
{
    std::vector<unsigned char> decoded;
    if (!DecodeBase58(privateKey, decoded, 64) || decoded.size() != 64) {
        throw std::invalid_argument("Invalid private key format");
    }

    std::memset(&node_, 0, sizeof(HDNode));

    std::memcpy(node_.private_key, decoded.data(), 32);

    uint8_t expected_public_key[32];
    ed25519_publickey(node_.private_key, expected_public_key);

    if (std::memcmp(expected_public_key, decoded.data() + 32, 32) != 0) {
        throw std::invalid_argument("Invalid private key: public key mismatch");
    }

    seed_.clear();
    mnemonic_.clear();
}

std::string SolanaWallet::deriveAddress(uint32_t index)
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

std::string SolanaWallet::derivePrivateKey(uint32_t index)
{
    if (!mnemonic_.empty())
        initNode(index);

    std::vector<uint8_t> full_private_key(64);
    uint8_t public_key[32];

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

Wallet::KeyPair SolanaWallet::deriveKeyPair(uint32_t index)
{
    // Todo
    return {
        std::string(),
        std::string(),
    };
}

std::vector<uint8_t> SolanaWallet::signMessage(std::span<const uint8_t> message)
{
    return std::vector<uint8_t>();
}

std::vector<uint8_t> SolanaWallet::signTransaction(
    std::span<const uint8_t> transaction)
{
    return std::vector<uint8_t>();
}

void SolanaWallet::initNode(uint32_t index)
{
    if (hdnode_from_seed(seed_.data(), SEED_SIZE, "ed25519", &node_) != 1) {
        throw std::runtime_error("Failed to create HD node from seed.");
    }

    if (!solanaMode_) {
        uint32_t path[] = {
            44 | HARDENED,    // 44'
            501 | HARDENED,   // 501'
            index | HARDENED, // 0'
            0 | HARDENED      // 0'
        };

        if (hdnode_private_ckd_cached(
                &node_, path, sizeof(path) / sizeof(uint32_t), nullptr)
            != 1) {
            throw std::runtime_error("Failed to derive key path.");
        }
    }
}

void SolanaWallet::cleanup()
{
    // Todo
}

bool SolanaWallet::solanaMode() const
{
    return solanaMode_;
}

void SolanaWallet::setSolanaMode(bool newSolanaMode)
{
    solanaMode_ = newSolanaMode;
}

}
