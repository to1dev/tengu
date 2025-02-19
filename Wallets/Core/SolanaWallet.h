#ifndef SOLANAWALLET_H
#define SOLANAWALLET_H

#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include "Utils/Base58.hpp"

#include "ChainWallet.h"

namespace Daitengu::Wallets {

class SolanaWallet : public ChainWallet {
public:
    SolanaWallet(NetworkType network = NetworkType::MAINNET);

    bool solanaMode() const;
    void setSolanaMode(bool newSolanaMode);

    void fromPrivateKey(const std::string& privateKey) override;
    [[nodiscard]] std::string getAddress(std::uint32_t index = 0) override;
    [[nodiscard]] std::string getPrivateKey(std::uint32_t index = 0) override;
    [[nodiscard]] KeyPair deriveKeyPair(std::uint32_t index = 0) override;

protected:
    const std::map<NetworkType, ChainNetwork> networkConfigs_ = {
        {
            NetworkType::MAINNET,
            {
                NetworkType::MAINNET,
                0x00,
                "mainnet-beta",
                "",
            },
        },
        {
            NetworkType::TESTNET,
            {
                NetworkType::TESTNET,
                0x00,
                "testnet",
                "",
            },
        },
        {
            NetworkType::DEVNET,
            {
                NetworkType::DEVNET,
                0x00,
                "devnet",
                "",
            },
        },
    };

    void onNetworkChanged() override;

private:
    void initNode(std::uint32_t index = 0);

private:
    bool solanaMode_ { false };
};

}
#endif // SOLANAWALLET_H
