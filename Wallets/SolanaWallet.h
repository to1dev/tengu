#ifndef SOLANAWALLET_H
#define SOLANAWALLET_H

#include "Utils/Base58.hpp"

#include "Wallet.h"

namespace Daitengu::Wallets {

class SolanaWallet : public Wallet {
public:
    SolanaWallet(bool solanaMode = false);
    ~SolanaWallet();

    void fromPrivateKey(const std::string& privateKey) override;
    std::string deriveAddress(uint32_t index = 0) override;
    std::string derivePrivateKey(uint32_t index = 0) override;
    [[nodiscard]] KeyPair deriveKeyPair(uint32_t index = 0) override;
    std::vector<uint8_t> signMessage(std::span<const uint8_t> message) override;
    std::vector<uint8_t> signTransaction(
        std::span<const uint8_t> transaction) override;

    bool solanaMode() const;
    void setSolanaMode(bool newSolanaMode);

private:
    void initNode(uint32_t index = 0);
    void cleanup();

private:
    bool solanaMode_;
};

}
#endif // SOLANAWALLET_H
