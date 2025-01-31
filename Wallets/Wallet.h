#ifndef WALLET_H
#define WALLET_H

#include <iostream>
#include <string>
#include <vector>

#include "Mnemonic.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "izanagi/bip32.h"

#ifdef __cplusplus
}
#endif

namespace Daitengu::Wallets {

class Wallet {
public:
    Wallet();
    virtual ~Wallet() = default;

    virtual std::string generateMnemonic(int strength = 128);
    virtual void fromMnemonic(
        const std::string& mnemonic, const std::string& passphrase = "");

    virtual std::string deriveAddress(uint32_t account) = 0;
    virtual std::vector<uint8_t> signMessage(
        const std::vector<uint8_t>& message, uint32_t index)
        = 0;

    std::string mnemonic() const;

protected:
    static bool validateMnemonic(const std::string& mnemonic);

protected:
    std::vector<uint8_t> seed_;
    std::string mnemonic_;

private:
    virtual void cleanup() = 0;
};

}
#endif // WALLET_H
