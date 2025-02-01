#ifndef SOLANAWALLET_H
#define SOLANAWALLET_H

#include "Utils/Base58.h"
#include "Wallet.h"

using namespace Daitengu::Utils;

namespace Daitengu::Wallets {

class SolanaWallet : public Wallet {
public:
    SolanaWallet();
    ~SolanaWallet();

    void fromPrivateKey(const std::string& privateKey) override;
    std::string deriveAddress(uint32_t index = 0) override;
    std::string derivePrivateKey(uint32_t index = 0) override;
    [[nodiscard]] KeyPair deriveKeyPair(uint32_t index = 0) override;
    std::vector<uint8_t> signMessage(std::span<const uint8_t> message) override;
    std::vector<uint8_t> signTransaction(
        std::span<const uint8_t> transaction) override;

protected:
    void initNode(uint32_t index = 0);
    void cleanup() override;
};

}
#endif // SOLANAWALLET_H
