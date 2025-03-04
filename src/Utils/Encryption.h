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

#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <cstring>
#include <format>
#include <stdexcept>
#include <vector>

#include <QString>

#include <sodium.h>
#include <xxhash.h>

#include "Security/Security.h"

using namespace Daitengu::Security;

namespace Daitengu::Utils {

inline constexpr int DEFAULT_EASYHASH_SEED = 6978;

class Encryption {
public:
    Encryption();
    virtual ~Encryption() = default;

    static bool init();

    static std::string genRandomHash();
    static QString easyHash(
        const QString& input, unsigned long long seed = DEFAULT_EASYHASH_SEED);
    static std::string easyHash(const std::string& input,
        unsigned long long seed = DEFAULT_EASYHASH_SEED);
    static std::string generateStrongPassword(size_t length);

    static QString encryptText(
        const QString& plainText, const QString& password = defaultKey);
    static std::string encryptText(const std::string& plainText,
        const std::string& password = defaultKey);

    static QString decryptText(
        const QString& encodedText, const QString& password = defaultKey);
    static std::string decryptText(const std::string& encodedText,
        const std::string& password = defaultKey);
};

}
#endif // ENCRYPTION_H
