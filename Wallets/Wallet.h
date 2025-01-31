#ifndef WALLET_H
#define WALLET_H

#include <iostream>
#include <span>
#include <string>
#include <vector>

#include <sodium.h>

#include "Mnemonic.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "izanagi/bip32.h"

#ifdef __cplusplus
}
#endif

namespace Daitengu::Wallets {

class SecureBytes {
public:
    explicit SecureBytes(std::span<const uint8_t> data);
    explicit SecureBytes(size_t size);

    ~SecureBytes();

    SecureBytes(const SecureBytes&) = delete;
    SecureBytes& operator=(const SecureBytes&) = delete;

    SecureBytes(SecureBytes&& other) noexcept;
    SecureBytes& operator=(SecureBytes&& other) noexcept;

    void clear();

    std::span<const uint8_t> span() const
    {
        return std::span<const uint8_t>(bytes_);
    }

    std::span<uint8_t> span()
    {
        return std::span<uint8_t>(bytes_);
    }

    uint8_t* data()
    {
        return bytes_.data();
    }

    const uint8_t* data() const
    {
        return bytes_.data();
    }

    size_t size() const
    {
        return bytes_.size();
    }

private:
    std::vector<uint8_t> bytes_;
};

inline constexpr uint32_t HARDENED = 0x80000000;
inline constexpr size_t SEED_SIZE = 64;

class Wallet {
public:
    Wallet();
    virtual ~Wallet();

    Wallet(const Wallet&) = delete;
    Wallet& operator=(const Wallet&) = delete;

    Wallet(Wallet&&) noexcept = default;
    Wallet& operator=(Wallet&&) noexcept = default;

    [[nodiscard]] virtual std::string generateMnemonic(int strength = 128);
    virtual void fromMnemonic(
        const std::string& mnemonic, const std::string& passphrase = "");

    virtual std::string deriveAddress(uint32_t account) = 0;
    [[nodiscard]] virtual std::vector<uint8_t> signMessage(
        std::span<const uint8_t> message)
        = 0;
    [[nodiscard]] virtual std::vector<uint8_t> signTransaction(
        std::span<const uint8_t> transaction)
        = 0;

    std::string mnemonic() const;

    void secureErase();

protected:
    // std::vector<uint8_t> seed_;
    SecureBytes seed_;
    std::string mnemonic_;

private:
    virtual void cleanup() = 0;
};

}
#endif // WALLET_H
