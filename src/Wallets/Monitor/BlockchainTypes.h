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

#include <QString>
#include <QUrl>

namespace Daitengu::Wallets {

struct TokenInfo {
    QString tokenAddress;
    QString symbol;
    QString name;
    QString balance;
    int decimals;
    QUrl iconUrl;

    TokenInfo() = default;

    TokenInfo(QString address, QString sym, QString nm, QString bal, int dec,
        QUrl icon)
        : tokenAddress(std::move(address))
        , symbol(std::move(sym))
        , name(std::move(nm))
        , balance(std::move(bal))
        , decimals(dec)
        , iconUrl(std::move(icon))
    {
    }
};

}
