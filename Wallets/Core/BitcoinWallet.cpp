#include "BitcoinWallet.h"

namespace Daitengu::Wallets {

BitcoinWallet::BitcoinWallet(Network::Type network)
    : ChainWallet(ChainType::BITCOIN, network)
{
}

void BitcoinWallet::onNetworkChanged()
{
}

}
