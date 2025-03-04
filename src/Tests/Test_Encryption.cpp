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

#include <iostream>

#include <QDebug>

#include "catch_amalgamated.hpp"

#include "Utils/Encryption.h"

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
        QString plain = "Hello World!";
        QString encoded = Encryption::encryptText(plain);
        QString decoded = Encryption::decryptText(encoded);

        REQUIRE(decoded == plain);
    }
}

#endif
