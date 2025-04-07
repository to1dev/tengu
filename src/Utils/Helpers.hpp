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
#include <iomanip>
#include <iostream>
#include <locale>
#include <random>
#include <regex>
#include <sstream>
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

inline std::string trim(const std::string& s)
{
    std::string_view sv(s);
    const char* whitespace = " \t\n\r\f\v";
    auto start = sv.find_first_not_of(whitespace);
    if (start == std::string_view::npos) {
        return "";
    }
    auto end = sv.find_last_not_of(whitespace);
    return std::string(sv.substr(start, end - start + 1));
}

inline std::string simplified(const std::string& s)
{
    std::string_view sv(s);
    const char* whitespace = " \t\n\r\f\v";
    auto start = sv.find_first_not_of(whitespace);
    if (start == std::string_view::npos)
        return "";
    auto end = sv.find_last_not_of(whitespace);
    sv = sv.substr(start, end - start + 1);

    std::string result;
    result.reserve(sv.size());
    bool in_whitespace = false;
    for (char ch : sv) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            if (!in_whitespace) {
                result.push_back(' ');
                in_whitespace = true;
            }
        } else {
            result.push_back(ch);
            in_whitespace = false;
        }
    }
    return result;
}

namespace QFormat {
    inline QString formatPrice(double value, int precision = 8)
    {
        QLocale locale(QLocale::English);
        return locale.toString(value, 'f', precision);
    }
}

namespace CFormat {
    inline std::string formatPrice(double value, int precision = 8)
    {
        std::ostringstream oss;
        oss.imbue(std::locale("en_US.UTF-8"));
        oss << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }
}

}
#endif // HELPERS_HPP
