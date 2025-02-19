#ifndef BASEWALLET_H
#define BASEWALLET_H

#include <cstdint>
#include <stdexcept>
#include <string>

#include "BaseMnemonic.h"

#include "../Utils/SecureBytes.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "izanagi/bip32.h"

#ifdef __cplusplus
}
#endif

namespace Daitengu::Wallets {

inline constexpr std::uint32_t HARDENED = 0x80000000;
inline constexpr std::size_t SEED_SIZE = 64;

class BaseWallet {
public:
    struct KeyPair {
        std::string public_key;
        std::string private_key;
    };

    BaseWallet();
    virtual ~BaseWallet();

    BaseWallet(const BaseWallet&) = delete;
    BaseWallet& operator=(const BaseWallet&) = delete;

    BaseWallet(BaseWallet&&) noexcept = default;
    BaseWallet& operator=(BaseWallet&&) noexcept = default;

    [[nodiscard]] virtual std::string generateMnemonic(int strength = 128);
    virtual void fromMnemonic(
        const std::string& mnemonic, const std::string& passphrase = "");

    virtual void fromPrivateKey(const std::string& privateKey) = 0;
    [[nodiscard]] virtual std::string getAddress(std::uint32_t index = 0) = 0;
    [[nodiscard]] virtual std::string getPrivateKey(std::uint32_t index = 0)
        = 0;
    [[nodiscard]] virtual KeyPair deriveKeyPair(std::uint32_t index = 0) = 0;

    std::string mnemonic() const;

protected:
    SecureBytes seed_;
    std::string mnemonic_;
    HDNode node_;

    virtual void initNode(std::uint32_t index = 0) = 0;
    virtual void secureErase();
};

}
#endif // BASEWALLET_H
