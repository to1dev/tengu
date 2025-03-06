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
#include <string>
#include <vector>

#include <sodium.h>

static inline std::string BytesToHex(const unsigned char* data, size_t len)
{
    size_t hexLen = len * 2 + 1;
    std::string hex(hexLen, '\0');

    sodium_bin2hex(hex.data(), hexLen, data, len);

    hex.pop_back();
    return hex;
}

static inline std::string BytesToHex(const std::vector<unsigned char>& data)
{
    return BytesToHex(data.data(), data.size());
}

static inline bool HexToBytes(
    const std::string& hex, std::vector<unsigned char>& out)
{
    size_t maxBinLen = hex.size() / 2;
    out.resize(maxBinLen);

    size_t binLen = 0;
    int ret = sodium_hex2bin(out.data(), maxBinLen, hex.data(), hex.size(),
        nullptr, &binLen, nullptr);
    if (ret != 0) {
        return false;
    }
    out.resize(binLen);
    return true;
}

static inline std::vector<unsigned char> HexToBytes(const std::string& hex)
{
    std::vector<unsigned char> buf;
    if (!HexToBytes(hex, buf)) {
        throw std::runtime_error("Invalid hex string");
    }
    return buf;
}
