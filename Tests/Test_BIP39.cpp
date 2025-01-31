#ifdef USE_TEST

#include <format>
#include <iostream>

#include "catch_amalgamated.hpp"

#include "Wallets/SolanaWallet.h"

using namespace Daitengu::Wallets;

TEST_CASE("Generate bip39 mnemonic")
{

    SECTION("New wallet")
    {
        SolanaWallet wallet;
        // std::cout << wallet.generateMnemonic() << std::endl;
        wallet.fromMnemonic("velvet pink canoe patient razor retreat spike "
                            "outdoor struggle deliver raw obscure");
        for (int i = 0; i < 5; i++) {
            std::string address = wallet.deriveAddress(i);
            std::cout << "Solana " << i << ": " << address << std::endl;
        }
    }

    SECTION("BIP32 and BIP39")
    {
    }
}

#endif
