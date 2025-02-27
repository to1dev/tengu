#include "ChainWallet.h"

namespace Daitengu::Wallets {

ChainWallet::ChainWallet(ChainType chainType, Network::Type network)
    : chainType_(chainType)
    , currentNetwork_(network)
{
    initChainConfig();
}

void ChainWallet::switchNetwork(Network::Type network)
{
    currentNetwork_ = network;
}

Network::Type ChainWallet::currentNetwork() const
{
    return currentNetwork_;
}

const ChainNetwork& ChainWallet::getCurrentNetworkConfig() const
{
    auto it = networkConfigs_.find(currentNetwork_);
    if (it == networkConfigs_.end()) {
        throw std::runtime_error("Unsupported network type.");
    }
    return it->second;
}

void ChainWallet::initChainConfig()
{
    static const std::map<ChainType, ChainConfig> configs = {
        { ChainType::SOLANA,
            { 501, "ed25519", true,
                { 44 | HARDENED, 501 | HARDENED, 0 | HARDENED,
                    0 | HARDENED } } },
        { ChainType::ETHEREUM,
            {
                60, "secp256k1", false,
                { 44 | HARDENED, 60 | HARDENED, 0 | HARDENED, 0 } // base path
            } },
        { ChainType::BITCOIN,
            { 0, "secp256k1", false,
                { 84 | HARDENED, 0 | HARDENED, 0 | HARDENED, 0 }

            } }
    };

    auto it = configs.find(chainType_);
    if (it == configs.end()) {
        throw std::runtime_error("Unsupported chain type.");
    }

    config_ = it->second;
}

}
