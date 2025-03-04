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

#ifndef BASEMNEMONIC_H
#define BASEMNEMONIC_H

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#include "Errors.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#include "izanagi/bip39.h"

#ifdef __cplusplus
}
#endif

namespace Daitengu::Wallets {

class BaseMnemonic {
public:
    explicit BaseMnemonic() = default;

    [[nodiscard]] static std::string generate(int strength = 128);
    [[nodiscard]] static std::string fromData(
        const std::vector<std::uint8_t>& data);
    [[nodiscard]] static bool check(const std::string& mnemonic);
    [[nodiscard]] static std::vector<std::uint8_t> toBits(
        const std::string& mnemonic);
    [[nodiscard]] static std::vector<std::uint8_t> toSeed(
        const std::string& mnemonic, const std::string& passphrase = "",
        std::function<void(std::uint32_t, std::uint32_t)> progressCallback
        = nullptr);
    [[nodiscard]] static int findWord(const std::string& word);
    [[nodiscard]] static std::string completeWord(const std::string& prefix);
    [[nodiscard]] static std::string getWord(int index);
    [[nodiscard]] static std::uint32_t getWordCompletionMask(
        const std::string& prefix);
};

}
#endif // BASEMNEMONIC_H
