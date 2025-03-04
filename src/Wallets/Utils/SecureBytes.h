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

#ifndef SECUREBYTES_H
#define SECUREBYTES_H

#include <cstdint>
#include <vector>

#include <sodium.h>

namespace Daitengu::Wallets {

class SecureBytes {
public:
    SecureBytes() = default;
    explicit SecureBytes(std::size_t size);
    explicit SecureBytes(const std::vector<std::uint8_t>& data);

    ~SecureBytes();

    SecureBytes(const SecureBytes&) = delete;
    SecureBytes& operator=(const SecureBytes&) = delete;

    SecureBytes(SecureBytes&& other) noexcept;
    SecureBytes& operator=(SecureBytes&& other) noexcept;

    void clear();

    const std::vector<std::uint8_t>& vec() const
    {
        return bytes_;
    }

    std::vector<std::uint8_t>& vec()
    {
        return bytes_;
    }

    std::uint8_t* data()
    {
        return bytes_.data();
    }

    const std::uint8_t* data() const
    {
        return bytes_.data();
    }

    std::size_t size() const
    {
        return bytes_.size();
    }

private:
    std::vector<std::uint8_t> bytes_;
};

}
#endif // SECUREBYTES_H
