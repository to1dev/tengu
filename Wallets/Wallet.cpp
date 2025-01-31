#include "Wallet.h"

namespace Daitengu::Wallets {

Wallet::Wallet()
{
}

std::string Wallet::generateMnemonic(int strength)
{
    try {
        cleanup();

        std::string mnemonic = Mnemonic::generate(strength);
        if (mnemonic.empty()) {
            throw std::runtime_error("Failed to generate mnemonic");
        }

        if (!validateMnemonic(mnemonic)) {
            throw std::runtime_error("Generated mnemonic validation failed");
        }

        std::vector<uint8_t> _seed = Mnemonic::toSeed(mnemonic);
        if (_seed.empty()) {
            throw std::runtime_error("Failed to generate seed from mnemonic");
        }

        seed_ = std::move(_seed);

        return mnemonic;
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

    if (!validateMnemonic(mnemonic)) {
        throw std::invalid_argument("Invalid mnemonic");
    }

    try {
        cleanup();

        std::vector<uint8_t> _seed = Mnemonic::toSeed(mnemonic, passphrase);
        if (_seed.empty()) {
            throw std::runtime_error("Failed to generate seed from mnemonic");
        }
        seed_ = std::move(_seed);
    } catch (const std::exception& e) {
        seed_.clear();
        throw;
    }
}

bool Wallet::validateMnemonic(const std::string& mnemonic)
{
    return mnemonic_check(mnemonic.c_str()) != 0;
}

std::string Wallet::mnemonic() const
{
    return mnemonic_;
}

}
