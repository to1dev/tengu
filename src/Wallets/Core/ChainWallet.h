#ifndef CHAINWALLET_H
#define CHAINWALLET_H

#include <map>
#include <stdexcept>

#include "BaseWallet.h"
#include "Types.h"

namespace Daitengu::Wallets {

class ChainWallet : public BaseWallet {
public:
    explicit ChainWallet(
        ChainType chainType, Network::Type network = Network::Type::MAINNET);

    void switchNetwork(Network::Type network);

    Network::Type currentNetwork() const;

    const ChainNetwork& getCurrentNetworkConfig() const;

protected:
    ChainType chainType_;
    ChainConfig config_;
    Network::Type currentNetwork_;
    std::map<Network::Type, ChainNetwork> networkConfigs_;

    virtual void onNetworkChanged() = 0;

private:
    void initChainConfig();
};

}
#endif // CHAINWALLET_H
