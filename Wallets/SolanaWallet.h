#ifndef SOLANAWALLET_H
#define SOLANAWALLET_H

#include <sodium.h>

#include "Utils/Base58.h"
#include "Wallet.h"

using namespace Daitengu::Utils;

namespace Daitengu::Wallets {

#define HARDENED 0x80000000

class SolanaWallet : public Wallet {
public:
    SolanaWallet();
    ~SolanaWallet();

    std::string deriveAddress(uint32_t index) override;
    std::vector<uint8_t> signMessage(
        const std::vector<uint8_t>& message, uint32_t index) override;

private:
    void cleanup() override;

private:
    HDNode node_;
};

}
#endif // SOLANAWALLET_H
