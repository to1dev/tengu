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

#include <memory>
#include <optional>

#include <QNetworkAccessManager>
#include <QObject>

#include <qcoro5/qcoro/QCoro>

#include "Monitor.h"
#include "Wallets/Core/Types.h"

namespace Daitengu::Wallets {

class Handler {
public:
    virtual ~Handler() = default;
    virtual QCoro::Task<std::optional<Monitor::BalanceResult>> fetchBalance(
        QNetworkAccessManager* networkManager, const QString& address)
        = 0;
    virtual void setPreferredApiSource(const QString& apiEndpoint) = 0;
    virtual QString getCurrentApiSource() const = 0;
};
}
