#ifndef SOLANAWALLET_H
#define SOLANAWALLET_H

#include "Utils/Base58.h"
#include "Wallet.h"

using namespace Daitengu::Utils;

namespace Daitengu::Wallets {

inline constexpr uint32_t HARDENED = 0x80000000;
inline constexpr size_t SEED_SIZE = 64;

class SolanaWallet : public Wallet {
public:
    SolanaWallet();
    ~SolanaWallet();

    std::string deriveAddress(uint32_t index) override;
    std::vector<uint8_t> signMessage(
        const std::vector<uint8_t>& message, uint32_t index) override;

private:
    void cleanup() override;
};

}
#endif // SOLANAWALLET_H
