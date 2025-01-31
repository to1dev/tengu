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

    hdnode_from_seed(seed_.data(), 64, "ed25519", &node);

    uint32_t path[] = {
        44 | HARDENED,    // 44'
        501 | HARDENED,   // 501'
        index | HARDENED, // 0'
        0 | HARDENED      // 0'
    };

    uint32_t fingerprint;
    hdnode_private_ckd_cached(
        &node, path, sizeof(path) / sizeof(uint32_t), &fingerprint);

    uint8_t public_key[32];
    ed25519_publickey(node.private_key, public_key);

    std::span<const unsigned char> pubkey_span(public_key, 32);
    return EncodeBase58(pubkey_span);
}

std::vector<uint8_t> SolanaWallet::signMessage(
    const std::vector<uint8_t>& message, uint32_t index)
{
    return std::vector<uint8_t>();
}

void SolanaWallet::cleanup()
{
    mnemonic_.clear();
    seed_.clear();
    memset(&node_, 0, sizeof(HDNode));
}

}
