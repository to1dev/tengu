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

#include "ScreenManager.h"

namespace Daitengu::Layouts {

ScreenManager::ScreenManager(QObject* parent)
    : QObject(parent)
{
    screens_ = QGuiApplication::screens();

    connect(qApp, &QGuiApplication::screenAdded, this,
        &ScreenManager::handleScreenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this,
        &ScreenManager::handleScreenRemoved);
}

QList<QScreen*> ScreenManager::screens() const
{
    return screens_;
}

void ScreenManager::handleScreenAdded(QScreen* screen)
{
    if (!screens_.contains(screen)) {
        screens_.append(screen);
        Q_EMIT screensChanged();
    }
}

void ScreenManager::handleScreenRemoved(QScreen* screen)
{
    if (screens_.contains(screen)) {
        screens_.removeAll(screen);
        Q_EMIT screensChanged();
    }
}

}
