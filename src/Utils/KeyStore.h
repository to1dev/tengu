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

#pragma once

#include <stdexcept>

#include <QByteArray>
#include <QString>

#include <sodium.h>

#include <comdef.h>
#include <dpapi.h>
#include <wincred.h>
#include <windows.h>

namespace Daitengu::Utils {

class KeyStore {
public:
    explicit KeyStore(const QString& appName = "Tengu");
    ~KeyStore() = default;

    void storeKey(const QByteArray& keyData, bool useCredentialManager = true);

    QByteArray readKey(bool useCredentialManager = true);

    QByteArray generateAndStoreKey(bool useCredentialManager = true);

    bool exportKey(const QString& filePath, const QString& password);

    bool importKey(const QString& filePath, const QString& password);

private:
    bool storeKeyToCredentialManager(const QByteArray& keyData);
    QByteArray readKeyFromCredentialManager();

    QByteArray encryptWithDPAPI(
        const QByteArray& data, const QByteArray& entropy = QByteArray());
    QByteArray decryptWithDPAPI(
        const QByteArray& encrypted, const QByteArray& entropy = QByteArray());
    void storeKeyToDPAPIFile(const QByteArray& keyData);
    QByteArray readKeyFromDPAPIFile();

    QString appName;
    QString keyFilePath;
};
}
