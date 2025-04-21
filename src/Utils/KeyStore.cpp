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

#include "KeyStore.h"

#include <cstring>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

namespace Daitengu::Utils {

KeyStore::KeyStore(const QString& appName)
    : appName(appName)
{
    QString appDataDir
        = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    keyFilePath = appDataDir + "/tengu_key.bin";
}

void KeyStore::storeKey(const QByteArray& keyData, bool useCredentialManager)
{
    if (keyData.size() != crypto_secretbox_KEYBYTES) {
        throw std::runtime_error("Invalid key size");
    }

    if (useCredentialManager) {
        if (!storeKeyToCredentialManager(keyData)) {
            throw std::runtime_error(
                "Failed to store key to Credential Manager");
        }
    } else {
        storeKeyToDPAPIFile(keyData);
    }
}

QByteArray KeyStore::readKey(bool useCredentialManager)
{
    QByteArray keyData;
    if (useCredentialManager) {
        keyData = readKeyFromCredentialManager();
    } else {
        keyData = readKeyFromDPAPIFile();
    }

    if (keyData.size() != crypto_secretbox_KEYBYTES) {
        throw std::runtime_error("Invalid key data");
    }

    return keyData;
}

QByteArray KeyStore::generateAndStoreKey(bool useCredentialManager)
{
    if (sodium_init() < 0) {
        throw std::runtime_error("Failed to initialize libsodium");
    }

    unsigned char key[crypto_secretbox_KEYBYTES];
    randombytes_buf(key, sizeof key);
    QByteArray keyData((char*)key, sizeof key);

    storeKey(keyData, useCredentialManager);
    return keyData;
}

bool KeyStore::exportKey(const QString& filePath, const QString& password)
{
    try {
        QByteArray keyData = readKey(true);
        if (keyData.isEmpty()) {
            keyData = readKey(false);
        }

        unsigned char salt[crypto_pwhash_SALTBYTES];
        randombytes_buf(salt, sizeof salt);
        unsigned char derivedKey[crypto_secretbox_KEYBYTES];
        if (crypto_pwhash(derivedKey, sizeof derivedKey,
                password.toUtf8().constData(), password.size(), salt,
                crypto_pwhash_OPSLIMIT_SENSITIVE,
                crypto_pwhash_MEMLIMIT_SENSITIVE, crypto_pwhash_ALG_DEFAULT)
            != 0) {
            return false;
        }

        unsigned char nonce[crypto_secretbox_NONCEBYTES];
        randombytes_buf(nonce, sizeof nonce);
        std::vector<unsigned char> cipherText(
            keyData.size() + crypto_secretbox_MACBYTES);
        crypto_secretbox_easy(cipherText.data(),
            (unsigned char*)keyData.constData(), keyData.size(), nonce,
            derivedKey);

        QByteArray exportData;
        exportData.append((char*)nonce, sizeof nonce);
        exportData.append((char*)salt, sizeof salt);
        exportData.append((char*)cipherText.data(), cipherText.size());

        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(exportData);
            file.setPermissions(QFile::ReadOwner | QFile::WriteOwner);
            file.close();
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        qDebug() << "Export key failed:" << e.what();
        return false;
    }
}

bool KeyStore::importKey(const QString& filePath, const QString& password)
{
    try {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            return false;
        }

        QByteArray importData = file.readAll();
        file.close();

        if (importData.size() < crypto_secretbox_NONCEBYTES
                + crypto_pwhash_SALTBYTES + crypto_secretbox_MACBYTES) {
            return false;
        }

        unsigned char nonce[crypto_secretbox_NONCEBYTES];
        unsigned char salt[crypto_pwhash_SALTBYTES];
        std::memcpy(nonce, importData.constData(), sizeof nonce);
        std::memcpy(salt, importData.constData() + sizeof nonce, sizeof salt);
        QByteArray cipherText = importData.mid(sizeof nonce + sizeof salt);

        unsigned char derivedKey[crypto_secretbox_KEYBYTES];
        if (crypto_pwhash(derivedKey, sizeof derivedKey,
                password.toUtf8().constData(), password.size(), salt,
                crypto_pwhash_OPSLIMIT_SENSITIVE,
                crypto_pwhash_MEMLIMIT_SENSITIVE, crypto_pwhash_ALG_DEFAULT)
            != 0) {
            return false;
        }

        QByteArray keyData(cipherText.size() - crypto_secretbox_MACBYTES, 0);
        if (crypto_secretbox_open_easy((unsigned char*)keyData.data(),
                (unsigned char*)cipherText.constData(), cipherText.size(),
                nonce, derivedKey)
            != 0) {
            return false;
        }

        storeKey(keyData, true);
        return true;
    } catch (const std::exception& e) {
        qDebug() << "Import key failed:" << e.what();
        return false;
    }
}

bool KeyStore::storeKeyToCredentialManager(const QByteArray& keyData)
{
    CREDENTIALW cred = { 0 };
    cred.Type = CRED_TYPE_GENERIC;
    cred.TargetName = (LPWSTR)(appName + "_EncryptionKey").utf16();
    cred.CredentialBlobSize = keyData.size();
    cred.CredentialBlob = (LPBYTE)keyData.constData();
    cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
    cred.UserName = (LPWSTR)L"TenguKey";

    if (CredWriteW(&cred, 0)) {
        return true;
    } else {
        qDebug() << "CredWriteW failed:" << GetLastError();
        return false;
    }
}

QByteArray KeyStore::readKeyFromCredentialManager()
{
    PCREDENTIALW cred;
    if (CredReadW((LPCWSTR)(appName + "_EncryptionKey").utf16(),
            CRED_TYPE_GENERIC, 0, &cred)) {
        QByteArray keyData(
            (char*)cred->CredentialBlob, cred->CredentialBlobSize);
        CredFree(cred);
        return keyData;
    } else {
        qDebug() << "CredReadW failed:" << GetLastError();
        return QByteArray();
    }
}

QByteArray KeyStore::encryptWithDPAPI(
    const QByteArray& data, const QByteArray& entropy)
{
    DATA_BLOB dataIn, dataOut, entropyBlob;
    dataIn.pbData = (BYTE*)data.constData();
    dataIn.cbData = data.size();
    entropyBlob.pbData
        = entropy.isEmpty() ? nullptr : (BYTE*)entropy.constData();
    entropyBlob.cbData = entropy.size();

    if (CryptProtectData(&dataIn, L"TenguKey",
            entropy.isEmpty() ? nullptr : &entropyBlob, nullptr, nullptr, 0,
            &dataOut)) {
        QByteArray encrypted((char*)dataOut.pbData, dataOut.cbData);
        LocalFree(dataOut.pbData);
        return encrypted;
    } else {
        qDebug() << "CryptProtectData failed:" << GetLastError();
        return QByteArray();
    }
}

QByteArray KeyStore::decryptWithDPAPI(
    const QByteArray& encrypted, const QByteArray& entropy)
{
    DATA_BLOB dataIn, dataOut, entropyBlob;
    dataIn.pbData = (BYTE*)encrypted.constData();
    dataIn.cbData = encrypted.size();
    entropyBlob.pbData
        = entropy.isEmpty() ? nullptr : (BYTE*)entropy.constData();
    entropyBlob.cbData = entropy.size();

    if (CryptUnprotectData(&dataIn, nullptr,
            entropy.isEmpty() ? nullptr : &entropyBlob, nullptr, nullptr, 0,
            &dataOut)) {
        QByteArray decrypted((char*)dataOut.pbData, dataOut.cbData);
        LocalFree(dataOut.pbData);
        return decrypted;
    } else {
        qDebug() << "CryptUnprotectData failed:" << GetLastError();
        return QByteArray();
    }
}

void KeyStore::storeKeyToDPAPIFile(const QByteArray& keyData)
{
    QByteArray encryptedKey = encryptWithDPAPI(keyData);
    if (encryptedKey.isEmpty()) {
        throw std::runtime_error("DPAPI encryption failed");
    }

    QFile file(keyFilePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(encryptedKey);
        file.setPermissions(QFile::ReadOwner | QFile::WriteOwner);
        file.close();
    } else {
        throw std::runtime_error(
            "Failed to write key file: " + file.errorString().toStdString());
    }
}

QByteArray KeyStore::readKeyFromDPAPIFile()
{
    QFile file(keyFilePath);
    if (!file.exists()) {
        return QByteArray();
    }

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray encryptedKey = file.readAll();
        file.close();
        QByteArray keyData = decryptWithDPAPI(encryptedKey);
        return keyData;
    } else {
        throw std::runtime_error(
            "Failed to read key file: " + file.errorString().toStdString());
    }
}
}
