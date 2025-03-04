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

#include "BaseWallet.h"

namespace Daitengu::Wallets {

BaseWallet::BaseWallet()
    : seed_(SEED_SIZE)
{
    if (sodium_init() < 0) {
        throw std::runtime_error("libsodium initialization failed.");
    }
}

BaseWallet::~BaseWallet()
{
    secureErase();
}

std::string BaseWallet::generateMnemonic(int strength)
{
    try {
        mnemonic_ = BaseMnemonic::generate(strength);
        if (mnemonic_.empty()) {
            throw std::runtime_error("Failed to generate mnemonic.");
        }
        fromMnemonic(mnemonic_);
        return mnemonic_;
    } catch (const std::exception& e) {
        seed_.clear();
        throw;
    }
}

void BaseWallet::fromMnemonic(
    const std::string& mnemonic, const std::string& passphrase)
{

    if (mnemonic.empty()) {
        throw std::invalid_argument("Mnemonic cannot be empty.");
    }

    if (!BaseMnemonic::check(mnemonic)) {
        throw std::invalid_argument("Invalid mnemonic.");
    }

    try {
        mnemonic_ = mnemonic;
        std::vector<uint8_t> seed = BaseMnemonic::toSeed(mnemonic_, passphrase);
        if (seed.empty()) {
            throw std::runtime_error("Failed to generate seed from mnemonic.");
        }

        // seed_ = std::move(seed);
        std::copy(seed.begin(), seed.end(), seed_.data());
        sodium_memzero(seed.data(), seed.size());

    } catch (const std::exception& e) {
        seed_.clear();
        throw;
    }
}

std::string BaseWallet::mnemonic() const
{
    return mnemonic_;
}

void BaseWallet::secureErase()
{
    seed_.clear();
    mnemonic_.clear();
    sodium_memzero(&node_, sizeof(HDNode));
}

}
