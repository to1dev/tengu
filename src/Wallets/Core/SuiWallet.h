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

#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include <sodium.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "izanagi/segwit_addr.h"

#ifdef __cplusplus
}
#endif

#include "ChainWallet.h"

namespace Daitengu::Wallets {

class SuiWallet : public ChainWallet {
public:
    static inline constexpr std::string_view DEFAULT_DERIVATION_PATH
        = "m/86'/0'/0'";

    std::string_view getDerivationPath() const override
    {
        return DEFAULT_DERIVATION_PATH;
    }

    explicit SuiWallet(Network::Type network = Network::Type::MAINNET);

    void fromPrivateKey(const std::string& privateKey) override;
    [[nodiscard]] std::string getAddress(std::uint32_t index = 0) override;
    [[nodiscard]] std::string getPrivateKey(std::uint32_t index = 0) override;
    [[nodiscard]] KeyPair deriveKeyPair(std::uint32_t index = 0) override;

protected:
    void onNetworkChanged() override;

private:
    void initNode(std::uint32_t index = 0) override;

    [[nodiscard]] std::string generateSuiAddress() const;

private:
    static constexpr unsigned char SCHEME_ED25519 = 0x00;
    static constexpr const char* SUI_PRIVATE_KEY_PREFIX = "suiprivkey";
    static constexpr const char* SUI_PRIVATE_KEY_BECH32_PREFIX = "suiprivkey1";
};

}
