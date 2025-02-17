#ifdef USE_TEST

#include <format>
#include <iostream>

#include "catch_amalgamated.hpp"

#include "Utils/Dotenv.hpp"
#include "Utils/PathUtils.hpp"

#include "Wallets/SolanaWallet.h"

using namespace Daitengu::Utils;
using namespace Daitengu::Wallets;

TEST_CASE("Generate bip39 mnemonic")
{
    SECTION("New wallet")
    {
        SolanaWallet wallet;
        std::cout << "Decent mnemonic: " << wallet.generateMnemonic()
                  << std::endl;
        std::cout << "Here is the new pubkey: " << wallet.deriveAddress()
                  << std::endl;
        std::cout << "Here is the new privkey: " << wallet.derivePrivateKey()
                  << std::endl;

        auto currentPath = PathUtils::getExecutableDir();
        auto& parser = DotEnv::getInstance();
        parser.load((currentPath / ".env").string());

        wallet.fromMnemonic(*parser.get("MNEMONIC"));
        std::cout << wallet.deriveAddress() << std::endl;
        std::cout << wallet.derivePrivateKey() << std::endl;

        /*REQUIRE(wallet.deriveAddress(0)
            == "9uvC3PMMzX4DgGrxDmheXNkMRWVfYqLsVpQjAaTD2uAp");
        REQUIRE(wallet.deriveAddress(1)
            == "AW48yjkZVi8JrTAywbC2DQA8hri44bmJfTZErK9FWRvU");
        REQUIRE(wallet.deriveAddress(2)
            == "HG1PSKC2SNGpHaramM3uXW7f8gZZ4cFA93PwG1RFfX8z");
        REQUIRE(wallet.deriveAddress(3)
            == "2Qi9BTPUNUxWD7jXBs9rU63YyusRQciVVfRF29pgr9Z6");
        REQUIRE(wallet.deriveAddress(4)
            == "4K83qTDQjEfSwj1qGjNoGxUydHyWhfoeLhuRzB9Kkp3t");*/
    }
}

#endif
