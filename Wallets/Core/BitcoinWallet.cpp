#include "BitcoinWallet.h"

namespace Daitengu::Wallets {

BitcoinWallet::BitcoinWallet(NetworkType network)
    : ChainWallet(ChainType::BITCOIN, network)
{
}

void BitcoinWallet::onNetworkChanged()
{
}

}
