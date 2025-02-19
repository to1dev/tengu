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
        ChainType chainType, NetworkType network = NetworkType::MAINNET);

    void switchNetwork(NetworkType network);

    NetworkType currentNetwork() const;

    const ChainNetwork& getCurrentNetworkConfig() const;

protected:
    ChainType chainType_;
    ChainConfig config_;
    NetworkType currentNetwork_;
    std::map<NetworkType, ChainNetwork> networkConfigs_;

    virtual void onNetworkChanged() = 0;

private:
    void initChainConfig();
};

}
#endif // CHAINWALLET_H
