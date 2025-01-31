#include "Wallet.h"

namespace Daitengu::Wallets {

Wallet::Wallet()
{
}

std::string Wallet::generateMnemonic(int strength)
{
    std::string mnemonic = Mnemonic::generate(strength);
    seed = Mnemonic::toSeed(mnemonic);

    return mnemonic;
}

void Wallet::fromMnemonic(const std::string& mnemonic)
{
    seed = Mnemonic::toSeed(mnemonic);
}

}
