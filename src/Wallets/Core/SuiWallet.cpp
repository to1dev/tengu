#include "SuiWallet.h"

namespace Daitengu::Wallets {

SuiWallet::SuiWallet(Network::Type network)
    : ChainWallet(ChainType::SUI, network)
{
}

}
