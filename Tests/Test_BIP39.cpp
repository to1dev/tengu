#ifdef USE_TEST

#include <format>
#include <iostream>

#include "catch_amalgamated.hpp"

#include "Utils/Dotenv.hpp"
#include "Wallets/SolanaWallet.h"

using namespace Daitengu::Wallets;

TEST_CASE("Generate bip39 mnemonic")
{

    SECTION("New wallet")
    {
        auto currentPath = DotEnv::getExePath();
        auto& parser = DotEnv::getInstance();
        parser.load((currentPath / ".env").string());

        SolanaWallet wallet;
        wallet.fromMnemonic(*parser.get("MNEMONIC"));
        REQUIRE(wallet.deriveAddress(0)
            == "9uvC3PMMzX4DgGrxDmheXNkMRWVfYqLsVpQjAaTD2uAp");
    }
}

#endif
