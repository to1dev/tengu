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

#include <QMap>
#include <QString>

#include "../Core/Types.h"

#include "BlockchainProvider.h"

namespace Daitengu::Wallets {

class ProviderFactory {
public:
    static BlockchainProvider* createProvider(ChainType chainType,
        ProviderType providerType, QObject* parent = nullptr);

    static QList<ProviderType> getAvailableProviders(ChainType chainType);

    static QString getProviderName(ProviderType type);
    static QString getProviderDescription(ProviderType type);

    static ProviderType getDefaultProvider(ChainType chainType);
};

}
