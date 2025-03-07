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

#include "BaseMnemonic.h"

namespace Daitengu::Wallets {

std::string BaseMnemonic::generate(int strength)
{
    if (strength != 128 && strength != 160 && strength != 192 && strength != 224
        && strength != 256) {
        throw std::invalid_argument(
            "Invalid strength value. Must be 128, 160, 192, 224, or 256.");
    }

    const char* result = mnemonic_generate(strength);
    if (!result) {
        throw MnemonicException("Failed to generate mnemonic.");
    }

    return std::string(result);
}

bool BaseMnemonic::check(const std::string& mnemonic)
{
    return mnemonic_check(mnemonic.c_str()) != 0;
}

int BaseMnemonic::findWord(const std::string& word)
{
    return mnemonic_find_word(word.c_str());
}

std::string BaseMnemonic::completeWord(const std::string& prefix)
{
    const char* result = mnemonic_complete_word(prefix.c_str(), prefix.size());

    if (!result) {
        throw std::runtime_error("Failed to complete word.");
    }

    return std::string(result);
}

std::string BaseMnemonic::getWord(int index)
{
    const char* result = mnemonic_get_word(index);

    if (!result) {
        throw std::runtime_error("Invalid word index.");
    }

    return std::string(result);
}

uint32_t BaseMnemonic::getWordCompletionMask(const std::string& prefix)
{
    return mnemonic_word_completion_mask(prefix.c_str(), prefix.size());
}

std::vector<uint8_t> BaseMnemonic::toSeed(const std::string& mnemonic,
    const std::string& passphrase,
    std::function<void(std::uint32_t, std::uint32_t)> progressCallback)
{
    std::vector<std::uint8_t> seed(512 / 8, 0);

    static std::function<void(std::uint32_t, std::uint32_t)> callback;
    callback = progressCallback;

    auto wrapper = [](std::uint32_t current, std::uint32_t total) {
        if (callback) {
            callback(current, total);
        }
    };

    mnemonic_to_seed(
        mnemonic.c_str(), passphrase.c_str(), seed.data(), wrapper);

    return seed;
}

std::vector<uint8_t> BaseMnemonic::toBits(const std::string& mnemonic)
{
    std::vector<std::uint8_t> bits(512 / 8, 0);

    if (mnemonic_to_bits(mnemonic.c_str(), bits.data()) == 0) {
        throw std::runtime_error("Failed to convert mnemonic to bits.");
    }

    return bits;
}

std::string BaseMnemonic::fromData(const std::vector<uint8_t>& data)
{
    const char* result = mnemonic_from_data(data.data(), data.size());

    if (!result) {
        throw std::runtime_error("Failed to generate mnemonic from data.");
    }

    mnemonic_clear();

    return std::string(result);
}

}
