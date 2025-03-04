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

#ifndef ERRORS_H
#define ERRORS_H

#include <stdexcept>
#include <string>

namespace Daitengu::Wallets {

class WalletException : public std::runtime_error {
public:
    explicit WalletException(const std::string& msg)
        : std::runtime_error("WalletException: " + msg)
    {
    }
};

class MnemonicException : public WalletException {
public:
    explicit MnemonicException(const std::string& msg)
        : WalletException("Mnemonic: " + msg)
    {
    }
};

class DatabaseException : public WalletException {
public:
    explicit DatabaseException(const std::string& msg)
        : WalletException("Database: " + msg)
    {
    }
};

}

#endif // ERRORS_H
