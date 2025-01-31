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
