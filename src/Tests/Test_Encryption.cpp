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

#include <iostream>

#include <QDebug>

#include "catch_amalgamated.hpp"

#include "Utils/Encryption.h"
#include "Utils/KeyStore.h"

using namespace Daitengu::Utils;

TEST_CASE("Random functions")
{
    SECTION("Generate random password")
    {
        QString hash1 = Encryption::easyHash(QString("Hello World!"));
        std::string hash2 = Encryption::easyHash(std::string("Hello World!"));
        REQUIRE(hash1.toStdString() == hash2);
    }
}

TEST_CASE("Encryption functions")
{
    SECTION("Encrypt and decrypt text")
    {
        KeyStore ks;

        QByteArray keyData = ks.readKey(true);
        REQUIRE(!keyData.isEmpty());

        Encryption enc;

        QString plain = "Hello World!";
        QString encoded = enc.encryptText(plain);
        QString decoded = enc.decryptText(encoded);

        std::string plain2 = Encryption::genRandomHash()
            + Encryption::generateStrongPassword(32);
        std::string encoded2 = enc.encryptText(plain2);
        std::string decoded2 = enc.decryptText(encoded2);

        REQUIRE(decoded == plain);
        REQUIRE(decoded2 == plain2);
    }
}

int main(int argc, char* argv[])
{
    int result = Catch::Session().run(argc, argv);

    return result;
}
