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
                "mainnet-beta",
                101,
                AddressEncoding::BASE58,
                "",
                {},
                "",
                0,
                false,
                nullptr,
                nullptr,
            },
        },
        {
            NetworkType::TESTNET,
            {
                NetworkType::TESTNET,
                "testnet",
                102,
                AddressEncoding::BASE58,
                "",
                {},
                "",
                0,
                false,
                nullptr,
                nullptr,
            },
        },
        {
            NetworkType::DEVNET,
            {
                NetworkType::DEVNET,
                "devnet",
                103,
                AddressEncoding::BASE58,
                "",
                {},
                "",
                0,
                false,
                nullptr,
                nullptr,
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
