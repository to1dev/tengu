#ifndef BITCOINWALLET_H
#define BITCOINWALLET_H

#include "ChainWallet.h"

namespace Daitengu::Wallets {

struct BitcoinNetwork : public Network {
    enum class Type {
        REGTEST = static_cast<int>(Network::Type::DEVNET) + 1,
        TEST4 = REGTEST + 1
    };
};

class BitcoinWallet : public ChainWallet {
public:
    BitcoinWallet(Network::Type network = Network::Type::MAINNET);

protected:
    const std::map<Network::Type, ChainNetwork> networkConfigs_ = {
        {
            Network::Type::MAINNET,
            {
                Network::Type::MAINNET,
                "mainnet",
                1,
                AddressEncoding::BECH32M,
                "",
                { 0x00 },
                "bc",
                0x00,
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
                2,
                AddressEncoding::BECH32M,
                "",
                { 0x6F },
                "tb",
                0x6F,
                false,
                nullptr,
                nullptr,
            },
        },
        {
            static_cast<Network::Type>(BitcoinNetwork::Type::REGTEST),
            {
                static_cast<Network::Type>(BitcoinNetwork::Type::REGTEST),
                "regtest",
                2,
                AddressEncoding::BECH32M,
                "",
                { 0x6F },
                "tb",
                0x6F,
                false,
                nullptr,
                nullptr,
            },
        },
    };

    void onNetworkChanged() override;
};

}
#endif // BITCOINWALLET_H
