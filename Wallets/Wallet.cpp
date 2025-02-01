#include "Wallet.h"

namespace Daitengu::Wallets {

SecureBytes::SecureBytes(std::span<const uint8_t> data)
    : bytes_(data.begin(), data.end())
{
}

SecureBytes::SecureBytes(size_t size)
    : bytes_(size)
{
}

SecureBytes::~SecureBytes()
{
    clear();
}

SecureBytes::SecureBytes(SecureBytes&& other) noexcept
    : bytes_(std::move(other.bytes_))
{
}

SecureBytes& SecureBytes::operator=(SecureBytes&& other) noexcept
{
    if (this != &other) {
        clear();
        bytes_ = std::move(other.bytes_);
    }

    return *this;
}

void SecureBytes::clear()
{
    if (!bytes_.empty()) {
        sodium_memzero(bytes_.data(), bytes_.size());
        bytes_.clear();
    }
}

Wallet::Wallet()
    : seed_(SEED_SIZE)
{
    if (sodium_init() < 0) {
        throw std::runtime_error("libsodium initialization failed.");
    }
}

Wallet::~Wallet()
{
    secureErase();
}

std::string Wallet::generateMnemonic(int strength)
{
    try {
        mnemonic_ = Mnemonic::generate(strength);
        if (mnemonic_.empty()) {
            throw std::runtime_error("Failed to generate mnemonic");
        }

        fromMnemonic(mnemonic_);

        return mnemonic_;
    } catch (const std::exception& e) {
        seed_.clear();
        throw;
    }
}

void Wallet::fromMnemonic(
    const std::string& mnemonic, const std::string& passphrase)
{
    if (mnemonic.empty()) {
        throw std::invalid_argument("Mnemonic cannot be empty");
    }

    if (!Mnemonic::check(mnemonic)) {
        throw std::invalid_argument("Invalid mnemonic");
    }

    try {
        mnemonic_ = mnemonic;
        std::vector<uint8_t> seed = Mnemonic::toSeed(mnemonic_, passphrase);
        if (seed.empty()) {
            throw std::runtime_error("Failed to generate seed from mnemonic");
        }

        // seed_ = std::move(seed);
        std::copy(seed.begin(), seed.end(), seed_.data());
        sodium_memzero(seed.data(), seed.size());

    } catch (const std::exception& e) {
        seed_.clear();
        throw;
    }
}

std::string Wallet::mnemonic() const
{
    return mnemonic_;
}

void Wallet::secureErase()
{
    seed_.clear();
    mnemonic_.clear();
    sodium_memzero(&node_, sizeof(HDNode));
}

}
