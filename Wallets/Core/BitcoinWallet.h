#ifndef BITCOINWALLET_H
#define BITCOINWALLET_H

#include "ChainWallet.h"

namespace Daitengu::Wallets {

class BitcoinWallet : public ChainWallet {
public:
    BitcoinWallet(NetworkType network = NetworkType::MAINNET);

protected:
    const std::map<NetworkType, ChainNetwork> networkConfigs_ = {
        {
            NetworkType::MAINNET,
            {
                NetworkType::MAINNET,
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
            NetworkType::TESTNET,
            {
                NetworkType::TESTNET,
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
    };

    void onNetworkChanged() override;
};

}
#endif // BITCOINWALLET_H
