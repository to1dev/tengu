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

#ifndef CHAINWALLET_H
#define CHAINWALLET_H

#include <map>
#include <stdexcept>

#include "BaseWallet.h"
#include "Types.h"

namespace Daitengu::Wallets {

class ChainWallet : public BaseWallet {
public:
    explicit ChainWallet(
        ChainType chainType, Network::Type network = Network::Type::MAINNET);

    virtual std::string_view getDerivationPath() const = 0;

    void switchNetwork(Network::Type network);

    Network::Type currentNetwork() const;

    const ChainNetwork& getCurrentNetworkConfig() const;

protected:
    ChainType chainType_;
    ChainConfig config_;
    Network::Type currentNetwork_;
    std::map<Network::Type, ChainNetwork> networkConfigs_;

    virtual void onNetworkChanged() = 0;

private:
    void initChainConfig();
};

}
#endif // CHAINWALLET_H
