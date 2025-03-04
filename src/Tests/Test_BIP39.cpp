// SPDX-License-Identifier: AGPL-3.0-or-later
/*
 * Copyright (C) 2025 to1dev <https://arc20.me/to1dev>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifdef USE_TEST

#include <format>
#include <iostream>

#include "catch_amalgamated.hpp"

#include "Utils/Dotenv.hpp"
#include "Utils/PathUtils.hpp"

#include "Wallets/Core/SolanaWallet.h"

using namespace Daitengu::Utils;
using namespace Daitengu::Wallets;

TEST_CASE("Generate bip39 mnemonic")
{
    SECTION("New wallet")
    {
        SolanaWallet wallet;
        std::cout << "Decent mnemonic: " << wallet.generateMnemonic()
                  << std::endl;
        std::cout << "Here is the new pubkey: " << wallet.getAddress()
                  << std::endl;
        std::cout << "Here is the new privkey: " << wallet.getPrivateKey()
                  << std::endl;

        auto currentPath = PathUtils::getExecutableDir();
        auto& parser = DotEnv::getInstance();
        parser.load((currentPath / ".env").string());

        wallet.fromMnemonic(*parser.get("MNEMONIC"));
        std::cout << wallet.getAddress() << std::endl;
        std::cout << wallet.getPrivateKey() << std::endl;

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
