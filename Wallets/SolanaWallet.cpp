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
    HDNode node;

    if (hdnode_from_seed(seed_.data(), SEED_SIZE, "ed25519", &node) != 1) {
        throw std::invalid_argument("Invalid seed.");
    }

    uint32_t path[] = {
        44 | HARDENED,    // 44'
        501 | HARDENED,   // 501'
        index | HARDENED, // 0'
        0 | HARDENED      // 0'
    };

    if (hdnode_private_ckd_cached(
            &node, path, sizeof(path) / sizeof(uint32_t), nullptr)
        != 1) {
        throw std::runtime_error("Failed to derive parent.");
    }

    uint8_t public_key[32];
    ed25519_publickey(node.private_key, public_key);

    std::span<const unsigned char> pubkey_span(public_key, 32);
    return EncodeBase58(pubkey_span);
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

void SolanaWallet::cleanup()
{
    mnemonic_.clear();
    seed_.clear();
}

}
