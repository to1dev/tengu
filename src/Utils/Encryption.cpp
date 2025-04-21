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

#include "Encryption.h"

namespace Daitengu::Utils {

Encryption::Encryption()
    : keyStore("Tengu")
{
}

bool Encryption::init()
{
    return sodium_init() != -1;
}

std::string Encryption::genRandomHash()
{
    if (sodium_init() < 0) {
        throw std::runtime_error("Failed to initialize libsodium");
    }

    constexpr size_t inputLength = 32;
    constexpr size_t hashLength = crypto_generichash_BYTES;

    unsigned char randomInput[inputLength];
    unsigned char hash[hashLength];

    randombytes_buf(randomInput, inputLength);

    if (crypto_generichash(
            hash, hashLength, randomInput, inputLength, nullptr, 0)
        != 0) {
        throw std::runtime_error("Failed to generate hash");
    }

    char hex[hashLength * 2 + 1] = { 0 };
    sodium_bin2hex(hex, sizeof(hex), hash, hashLength);

    return std::string(hex);
}

QString Encryption::easyHash(const QString& input, unsigned long long seed)
{
    QByteArray inputData = input.toUtf8();
    XXH64_hash_t hash = XXH64(inputData.constData(), inputData.size(), seed);

    return QString::number(hash, 16);
}

std::string Encryption::easyHash(
    const std::string& input, unsigned long long seed)
{
    XXH64_hash_t hash = XXH64(input.c_str(), input.size(), seed);

    return std::format("{:x}", hash);
}

std::string Encryption::generateStrongPassword(size_t length)
{
    if (sodium_init() < 0) {
        throw std::runtime_error("Failed to initialize libsodium");
    }

    constexpr std::string_view characters = "0123456789"
                                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                            "abcdefghijklmnopqrstuvwxyz"
                                            "!@#$%^&*()_+-=[]{}|;:,.<>?";

    if (length == 0) {
        throw std::invalid_argument("Password length must be greater than 0");
    }

    const auto charCount = static_cast<unsigned int>(characters.size());
    std::vector<char> password(length);

    std::generate(password.begin(), password.end(),
        [&]() -> char { return characters[randombytes_uniform(charCount)]; });

    return std::string(password.begin(), password.end());
}

QString Encryption::encryptText(
    const QString& plainText, const QString& password)
{
    if (plainText.isEmpty()) {
        return plainText;
    }

    if (sodium_init() < 0) {
        throw std::runtime_error("Failed to initialize libsodium");
    }

    QByteArray plainTextBytes = plainText.toUtf8();
    unsigned char key[crypto_secretbox_KEYBYTES];

    if (password.isEmpty()) {
        QByteArray keyData;
        try {
            keyData = keyStore.readKey(true);
        } catch (const std::exception& e) {
            qDebug() << "Credential Manager failed:" << e.what();
            try {
                keyData = keyStore.readKey(false);
            } catch (const std::exception& e2) {
                qDebug() << "DPAPI failed, generating new key:" << e2.what();
                keyData = keyStore.generateAndStoreKey(true);
            }
        }
        if (keyData.size() != crypto_secretbox_KEYBYTES) {
            throw std::runtime_error("Invalid key size");
        }
        std::memcpy(key, keyData.constData(), crypto_secretbox_KEYBYTES);
    } else {
        QByteArray passwordBytes = password.toUtf8();
        unsigned char salt[crypto_pwhash_SALTBYTES];
        randombytes_buf(salt, sizeof salt);
        if (crypto_pwhash(key, sizeof key, passwordBytes.constData(),
                passwordBytes.size(), salt, crypto_pwhash_OPSLIMIT_INTERACTIVE,
                crypto_pwhash_MEMLIMIT_INTERACTIVE, crypto_pwhash_ALG_DEFAULT)
            != 0) {
            throw std::runtime_error("Password hashing failed");
        }
    }

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof nonce);

    QByteArray cipherText(plainTextBytes.size() + crypto_secretbox_MACBYTES, 0);
    crypto_secretbox_easy((unsigned char*)cipherText.data(),
        (unsigned char*)plainTextBytes.constData(), plainTextBytes.size(),
        nonce, key);

    sodium_memzero(key, sizeof key);

    QByteArray encryptedData = QByteArray((char*)nonce, sizeof nonce);
    if (password.isEmpty()) {
        encryptedData.append(QByteArray(crypto_pwhash_SALTBYTES, 0));
    } else {
        unsigned char salt[crypto_pwhash_SALTBYTES];
        randombytes_buf(salt, sizeof salt);
        encryptedData.append((char*)salt, sizeof salt);
    }
    encryptedData.append(cipherText);

    return QString(encryptedData.toBase64());
}

std::string Encryption::base64Encode(const std::string& input)
{
    size_t encodedLength = sodium_base64_ENCODED_LEN(
        input.size(), sodium_base64_VARIANT_ORIGINAL);
    std::string encoded(encodedLength, '\0');

    sodium_bin2base64(&encoded[0], encoded.size(),
        reinterpret_cast<const unsigned char*>(input.data()), input.size(),
        sodium_base64_VARIANT_ORIGINAL);

    encoded.resize(encodedLength - 1);

    return encoded;
}

std::string Encryption::base64Decode(const std::string& input)
{
    size_t decodedLength = input.size();
    std::vector<unsigned char> decoded(decodedLength);

    if (sodium_base642bin(decoded.data(), decoded.size(), input.data(),
            input.size(), nullptr, &decodedLength, nullptr,
            sodium_base64_VARIANT_ORIGINAL)
        != 0) {
        throw std::runtime_error("Base64 decoding failed");
    }

    return std::string(reinterpret_cast<char*>(decoded.data()), decodedLength);
}

std::string Encryption::encryptText(
    const std::string& plainText, const std::string& password)
{
    if (plainText.empty()) {
        return plainText;
    }

    if (sodium_init() < 0) {
        throw std::runtime_error("Failed to initialize libsodium");
    }

    unsigned char key[crypto_secretbox_KEYBYTES];

    if (password.empty()) {
        QByteArray keyData;
        try {
            keyData = keyStore.readKey(true);
        } catch (const std::exception& e) {
            qDebug() << "Credential Manager failed:" << e.what();
            try {
                keyData = keyStore.readKey(false);
            } catch (const std::exception& e2) {
                qDebug() << "DPAPI failed, generating new key:" << e2.what();
                keyData = keyStore.generateAndStoreKey(true);
            }
        }
        if (keyData.size() != crypto_secretbox_KEYBYTES) {
            throw std::runtime_error("Invalid key size");
        }
        std::memcpy(key, keyData.constData(), crypto_secretbox_KEYBYTES);
    } else {
        unsigned char salt[crypto_pwhash_SALTBYTES];
        randombytes_buf(salt, sizeof salt);
        if (crypto_pwhash(key, sizeof key, password.data(), password.size(),
                salt, crypto_pwhash_OPSLIMIT_INTERACTIVE,
                crypto_pwhash_MEMLIMIT_INTERACTIVE, crypto_pwhash_ALG_DEFAULT)
            != 0) {
            throw std::runtime_error("Password hashing failed");
        }
    }

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof nonce);

    std::vector<unsigned char> cipherText(
        plainText.size() + crypto_secretbox_MACBYTES);
    crypto_secretbox_easy(cipherText.data(),
        (const unsigned char*)plainText.data(), plainText.size(), nonce, key);

    sodium_memzero(key, sizeof key);

    std::string encryptedData;
    encryptedData.reserve(
        sizeof nonce + crypto_pwhash_SALTBYTES + cipherText.size());
    encryptedData.append(reinterpret_cast<char*>(nonce), sizeof nonce);

    if (password.empty()) {
        encryptedData.append(crypto_pwhash_SALTBYTES, '\0');
    } else {
        unsigned char salt[crypto_pwhash_SALTBYTES];
        randombytes_buf(salt, sizeof salt);
        encryptedData.append(reinterpret_cast<char*>(salt), sizeof salt);
    }

    encryptedData.append(
        reinterpret_cast<char*>(cipherText.data()), cipherText.size());

    return base64Encode(encryptedData);
}

QString Encryption::decryptText(
    const QString& encodedText, const QString& password)
{
    if (encodedText.isEmpty()) {
        return encodedText;
    }

    if (sodium_init() < 0) {
        throw std::runtime_error("Failed to initialize libsodium");
    }

    QByteArray decodedData = QByteArray::fromBase64(encodedText.toUtf8());
    if (decodedData.size() < crypto_secretbox_NONCEBYTES
            + crypto_pwhash_SALTBYTES + crypto_secretbox_MACBYTES) {
        throw std::runtime_error("Invalid encoded data");
    }

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    unsigned char salt[crypto_pwhash_SALTBYTES];

    std::memcpy(nonce, decodedData.data(), sizeof nonce);
    std::memcpy(salt, decodedData.data() + sizeof nonce, sizeof salt);

    QByteArray cipherText = decodedData.mid(sizeof nonce + sizeof salt);

    unsigned char key[crypto_secretbox_KEYBYTES];

    if (password.isEmpty()) {
        QByteArray keyData = keyStore.readKey(true);
        if (keyData.isEmpty()) {
            keyData = keyStore.readKey(false);
        }
        if (keyData.size() != crypto_secretbox_KEYBYTES) {
            throw std::runtime_error("Invalid key size");
        }
        std::memcpy(key, keyData.constData(), crypto_secretbox_KEYBYTES);
    } else {
        if (crypto_pwhash(key, sizeof key, password.toUtf8().constData(),
                password.size(), salt, crypto_pwhash_OPSLIMIT_INTERACTIVE,
                crypto_pwhash_MEMLIMIT_INTERACTIVE, crypto_pwhash_ALG_DEFAULT)
            != 0) {
            throw std::runtime_error("Password hashing failed");
        }
    }

    QByteArray decryptedText(cipherText.size() - crypto_secretbox_MACBYTES, 0);
    if (crypto_secretbox_open_easy((unsigned char*)decryptedText.data(),
            (unsigned char*)cipherText.constData(), cipherText.size(), nonce,
            key)
        != 0) {
        throw std::runtime_error(
            "Decryption failed: invalid password or corrupted data");
    }

    sodium_memzero(key, sizeof key);

    return QString::fromUtf8(decryptedText);
}

std::string Encryption::decryptText(
    const std::string& encodedText, const std::string& password)
{
    if (encodedText.empty()) {
        return encodedText;
    }

    if (sodium_init() < 0) {
        throw std::runtime_error("Failed to initialize libsodium");
    }

    std::string decodedData = base64Decode(encodedText);
    if (decodedData.size() < crypto_secretbox_NONCEBYTES
            + crypto_pwhash_SALTBYTES + crypto_secretbox_MACBYTES) {
        throw std::runtime_error("Invalid encoded data");
    }

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    unsigned char salt[crypto_pwhash_SALTBYTES];

    std::memcpy(nonce, decodedData.data(), sizeof nonce);
    std::memcpy(salt, decodedData.data() + sizeof nonce, sizeof salt);

    std::vector<unsigned char> cipherText(
        decodedData.begin() + sizeof nonce + sizeof salt, decodedData.end());

    unsigned char key[crypto_secretbox_KEYBYTES];

    if (password.empty()) {
        QByteArray keyData = keyStore.readKey(true);
        if (keyData.isEmpty()) {
            keyData = keyStore.readKey(false);
        }
        if (keyData.size() != crypto_secretbox_KEYBYTES) {
            throw std::runtime_error("Invalid key size");
        }
        std::memcpy(key, keyData.constData(), crypto_secretbox_KEYBYTES);
    } else {
        if (crypto_pwhash(key, sizeof key, password.data(), password.size(),
                salt, crypto_pwhash_OPSLIMIT_INTERACTIVE,
                crypto_pwhash_MEMLIMIT_INTERACTIVE, crypto_pwhash_ALG_DEFAULT)
            != 0) {
            throw std::runtime_error("Password hashing failed");
        }
    }

    std::vector<unsigned char> decryptedText(
        cipherText.size() - crypto_secretbox_MACBYTES);
    if (crypto_secretbox_open_easy(decryptedText.data(), cipherText.data(),
            cipherText.size(), nonce, key)
        != 0) {
        throw std::runtime_error(
            "Decryption failed: invalid password or corrupted data");
    }

    sodium_memzero(key, sizeof key);

    return std::string(
        reinterpret_cast<char*>(decryptedText.data()), decryptedText.size());
}

bool Encryption::exportKey(const QString& filePath, const QString& password)
{
    if (!validatePassword(password)) {
        return false;
    }

    return keyStore.exportKey(filePath, password);
}

bool Encryption::importKey(const QString& filePath, const QString& password)
{
    if (!validatePassword(password)) {
        return false;
    }

    return keyStore.importKey(filePath, password);
}

bool Encryption::validatePassword(const QString& password)
{
    if (password.isEmpty()) {
        return true;
    }

    if (password.length() < 8) {
        qDebug() << "Password too short (< 8 characters)";
        return false;
    }

    bool hasUpper = false, hasLower = false, hasDigit = false,
         hasSpecial = false;

    for (const QChar& c : password) {
        if (c.isUpper())
            hasUpper = true;
        else if (c.isLower())
            hasLower = true;
        else if (c.isDigit())
            hasDigit = true;
        else if (!c.isLetterOrNumber())
            hasSpecial = true;
    }

    bool valid = hasUpper && hasLower && hasDigit && hasSpecial;
    if (!valid) {
        qDebug() << "Password must include uppercase, lowercase, digit, and "
                    "special character";
    }

    return valid;
}
}
