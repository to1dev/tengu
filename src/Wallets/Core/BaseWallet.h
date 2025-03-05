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

#ifndef BASEWALLET_H
#define BASEWALLET_H

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

#include <sodium.h>

#include "BaseMnemonic.h"
#include "Errors.hpp"

#include "../Utils/SecureBytes.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "izanagi/bip32.h"

#ifdef __cplusplus
}
#endif

namespace Daitengu::Wallets {

inline constexpr std::uint32_t HARDENED = 0x80000000;
inline constexpr std::size_t SEED_SIZE = 64;

class BaseWallet {
public:
    struct KeyPair {
        std::string public_key;
        std::string private_key;
    };

    explicit BaseWallet();
    virtual ~BaseWallet();

    BaseWallet(const BaseWallet&) = delete;
    BaseWallet& operator=(const BaseWallet&) = delete;

    BaseWallet(BaseWallet&&) noexcept = default;
    BaseWallet& operator=(BaseWallet&&) noexcept = default;

    [[nodiscard]] virtual std::string generateMnemonic(int strength = 128);
    virtual void fromMnemonic(
        const std::string& mnemonic, const std::string& passphrase = "");

    virtual void fromPrivateKey(const std::string& privateKey) = 0;
    [[nodiscard]] virtual std::string getAddress(std::uint32_t index = 0) = 0;
    [[nodiscard]] virtual std::string getPrivateKey(std::uint32_t index = 0)
        = 0;
    [[nodiscard]] virtual KeyPair deriveKeyPair(std::uint32_t index = 0) = 0;

    std::string mnemonic() const;

protected:
    SecureBytes seed_;
    std::string mnemonic_;
    HDNode node_;

    virtual void initNode(std::uint32_t index = 0) = 0;
    virtual void secureErase();
};

}
#endif // BASEWALLET_H
