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

#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <cstdint>
#include <cstring>
#include <iostream>
#include <random>
#include <regex>
#include <string>

#include <QLocale>
#include <QRegularExpression>
#include <QString>

namespace Daitengu::Utils {

inline int randomIndex(int start, int end)
{
    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<int> distribution(start, end);
    return distribution(generator);
}

inline std::string hideAddress(std::string_view address)
{
    if (address.length() <= 10) {
        return std::string(address);
    }

    return std::string(address.substr(0, 5)) + "..."
        + std::string(address.substr(address.size() - 5, 5));
}

inline QString hideAddress(const QString& address)
{
    if (address.length() <= 10) {
        return address;
    }

    return address.left(5) + "..." + address.right(5);
}

}
#endif // HELPERS_HPP
