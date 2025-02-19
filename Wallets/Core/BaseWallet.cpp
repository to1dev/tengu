#include "BaseWallet.h"

namespace Daitengu::Wallets {

BaseWallet::BaseWallet()
    : seed_(SEED_SIZE)
{
    if (sodium_init() < 0) {
        throw std::runtime_error("libsodium initialization failed.");
    }
}

BaseWallet::~BaseWallet()
{
    secureErase();
}

std::string BaseWallet::generateMnemonic(int strength)
{
    try {
        mnemonic_ = BaseMnemonic::generate(strength);
        if (mnemonic_.empty()) {
            throw std::runtime_error("Failed to generate mnemonic.");
        }
        fromMnemonic(mnemonic_);
        return mnemonic_;
    } catch (const std::exception& e) {
        seed_.clear();
        throw;
    }
}

void BaseWallet::fromMnemonic(
    const std::string& mnemonic, const std::string& passphrase)
{
    if (mnemonic_.empty()) {
        throw std::invalid_argument("Mnemonic cannot be empty.");
    }

    if (!BaseMnemonic::check(mnemonic)) {
        throw std::invalid_argument("Invalid mnemonic.");
    }

    try {
        mnemonic_ = mnemonic;
        std::vector<uint8_t> seed = BaseMnemonic::toSeed(mnemonic_, passphrase);
        if (seed.empty()) {
            throw std::runtime_error("Failed to generate seed from mnemonic.");
        }

        // seed_ = std::move(seed);
        std::copy(seed.begin(), seed.end(), seed_.data());
        sodium_memzero(seed.data(), seed.size());

    } catch (const std::exception& e) {
        seed_.clear();
        throw;
    }
}

std::string BaseWallet::mnemonic() const
{
    return mnemonic_;
}

void BaseWallet::secureErase()
{
    seed_.clear();
    mnemonic_.clear();
    sodium_memzero(&node_, sizeof(HDNode));
}

}
