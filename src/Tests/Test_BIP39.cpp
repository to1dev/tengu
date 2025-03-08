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

#ifndef USE_TEST
#define USE_TEST
#endif
#ifdef USE_TEST

#include <format>
#include <iostream>

#include "catch_amalgamated.hpp"

#include "Utils/Dotenv.hpp"
#include "Utils/PathUtils.hpp"

#include "Wallets/Core/BitcoinWallet.h"
#include "Wallets/Core/EthereumWallet.h"
#include "Wallets/Core/SolanaWallet.h"
#include "Wallets/Core/SuiWallet.h"

using namespace Daitengu::Utils;
using namespace Daitengu::Wallets;

TEST_CASE("Generate bip39 mnemonic")
{
    SECTION("New wallet")
    {
        SolanaWallet solWallet1;
        std::cout << std::format(
            "Decent solana mnemonic: {}\n", solWallet1.generateMnemonic());
        std::cout << std::format(
            "Here is the new solana pubkey: {}\n", solWallet1.getAddress());
        std::cout << std::format(
            "Here is the new solana privkey: {}\n", solWallet1.getPrivateKey());

        auto currentPath = PathUtils::getExecutableDir();
        auto& parser = DotEnv::getInstance();
        parser.load((currentPath / ".env").string());

        SolanaWallet solWallet2;

        solWallet2.fromMnemonic(*parser.get("SOL_MNEMONIC"));
        std::cout << std::format("solana fromMnemonic address: {}\nsolana "
                                 "fromMnemonic privkey: {}\n",
            solWallet2.getAddress(), solWallet2.getPrivateKey());

        SolanaWallet solWallet3;
        solWallet3.fromPrivateKey(*parser.get("SOL_PRIVATE_KEY"));
        std::cout << "solana fromPrivateKey address: "
                  << solWallet3.getAddress() << std::endl;

        BitcoinWallet btcWallet1;
        std::cout << std::format(
            "Decent bitcoin mnemonic: {}\n", btcWallet1.generateMnemonic());
        std::cout << std::format(
            "Here is the new bitcoin pubkey: {}\n", btcWallet1.getAddress());
        std::cout << std::format("Here is the new bitcoin privkey: {}\n",
            btcWallet1.getPrivateKey());
        std::cout << std::format("Here is the new bitcoin scriptPubKey: {}\n",
            btcWallet1.getScriptPubKey());
        std::cout << std::format("Here is the new bitcoin scriptHash: {}\n",
            btcWallet1.getScriptHash());

        BitcoinWallet btcWallet2;
        btcWallet2.fromMnemonic(*parser.get("BTC_MNEMONIC"));
        std::cout << "bitcoin fromMnemonic address: " << btcWallet2.getAddress()
                  << std::endl;
        std::cout << "bitcoin fromMnemonic privkey: "
                  << btcWallet2.getPrivateKey() << std::endl;

        BitcoinWallet btcWallet3;
        btcWallet3.fromPrivateKey(*parser.get("BTC_PRIVATE_KEY"));
        std::cout << "bitcoin fromPrivateKey address: "
                  << btcWallet3.getAddress() << std::endl;

        EthereumWallet ethWallet1;
        ethWallet1.fromMnemonic(*parser.get("ETH_MNEMONIC"));
        std::cout << "ethereum fromMnemonic address: "
                  << ethWallet1.getAddress() << std::endl;
        std::cout << "ethereum fromMnemonic privkey: "
                  << ethWallet1.getPrivateKey() << std::endl;

        EthereumWallet ethWallet2;
        ethWallet2.fromPrivateKey(*parser.get("ETH_PRIVATE_KEY"));
        std::cout << "ethereum fromPrivateKey address: "
                  << ethWallet2.getAddress() << std::endl;

        SuiWallet suiWallet1;
        std::cout << std::format(
            "Decent sui mnemonic: {}\n", suiWallet1.generateMnemonic());
        std::cout << std::format(
            "Here is the new sui pubkey: {}\n", suiWallet1.getAddress());
        std::cout << std::format(
            "Here is the new sui privkey {}\n", suiWallet1.getPrivateKey());

        SuiWallet suiWallet2;
        suiWallet2.fromMnemonic(*parser.get("SUI_MNEMONIC"));
        std::cout << "sui fromMnemonic address: " << suiWallet2.getAddress()
                  << std::endl;
        std::cout << "sui fromMnemonic privkey: " << suiWallet2.getPrivateKey()
                  << std::endl;

        SuiWallet suiWallet3;
        suiWallet2.fromPrivateKey(*parser.get("SUI_PRIVATE_KEY"));
        std::cout << "sui fromPrivateKey address: " << suiWallet3.getAddress()
                  << std::endl;

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
