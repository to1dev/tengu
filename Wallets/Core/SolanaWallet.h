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
    static inline constexpr std::string_view DEFAULT_DERIVATION_PATH
        = "m/44'/501'/0'";

public:
    SolanaWallet(Network::Type network = Network::Type::MAINNET);

    bool solanaMode() const;
    void setSolanaMode(bool newSolanaMode);

    void fromPrivateKey(const std::string& privateKey) override;
    [[nodiscard]] std::string getAddress(std::uint32_t index = 0) override;
    [[nodiscard]] std::string getPrivateKey(std::uint32_t index = 0) override;
    [[nodiscard]] KeyPair deriveKeyPair(std::uint32_t index = 0) override;

protected:
    const std::map<Network::Type, ChainNetwork> networkConfigs_ = {
        {
            Network::Type::MAINNET,
            {
                Network::Type::MAINNET,
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
            Network::Type::TESTNET,
            {
                Network::Type::TESTNET,
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
            Network::Type::DEVNET,
            {
                Network::Type::DEVNET,
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
