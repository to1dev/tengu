#include "SolanaWallet.h"

namespace Daitengu::Wallets {

SolanaWallet::SolanaWallet()
{
}

SolanaWallet::~SolanaWallet()
{
    cleanup();
}

std::string SolanaWallet::deriveAddress(uint32_t index)
{
    initNode(index);

    uint8_t public_key[32];
    ed25519_publickey(node_.private_key, public_key);

    std::span<const unsigned char> pubkey_span(public_key, 32);
    return EncodeBase58(pubkey_span);
}

std::string SolanaWallet::getPrivateKey(uint32_t index)
{
    initNode(index);

    std::vector<uint8_t> full_private_key(64);
    std::copy(
        node_.private_key, node_.private_key + 32, full_private_key.begin());

    uint8_t public_key[32];
    ed25519_publickey(node_.private_key, public_key);

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

void SolanaWallet::cleanup()
{
    // Todo
}

}
