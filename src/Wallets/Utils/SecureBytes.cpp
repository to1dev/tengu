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

#include "SecureBytes.h"

namespace Daitengu::Wallets {

SecureBytes::SecureBytes(const std::vector<uint8_t>& data)
    : bytes_(data)
{
}

SecureBytes::SecureBytes(std::size_t size)
    : bytes_(size, 0)
{
}

SecureBytes::~SecureBytes()
{
    clear();
}

SecureBytes::SecureBytes(SecureBytes&& other) noexcept
    : bytes_(std::move(other.bytes_))
{
    other.bytes_.clear();
}

SecureBytes& SecureBytes::operator=(SecureBytes&& other) noexcept
{
    if (this != &other) {
        clear();
        bytes_ = std::move(other.bytes_);
        other.bytes_.clear();
    }

    return *this;
}

void SecureBytes::clear()
{
    if (!bytes_.empty()) {
        sodium_memzero(bytes_.data(), bytes_.size());
        bytes_.clear();
    }
}

}
