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

#include "MonitorManager.h"

namespace Daitengu::Wallets {

MonitorManager::MonitorManager(QObject* parent)
{
}

MonitorManager::~MonitorManager()
{
}

QCoro::Task<void> MonitorManager::setAddress(const QString& address)
{
    return QCoro::Task<void>();
}

QCoro::Task<void> MonitorManager::refreshBalance()
{
    return QCoro::Task<void>();
}

QCoro::Task<void> MonitorManager::refreshTokens()
{
    return QCoro::Task<void>();
}

void MonitorManager::registerMonitor(ChainType type, BlockchainMonitor* monitor)
{
}

BlockchainMonitor* MonitorManager::getMonitor(ChainType chainType)
{
    return nullptr;
}

QCoro::Task<void> MonitorManager::setAutoRefreshInterval(int milliseconds)
{
    return QCoro::Task<void>();
}

QCoro::Task<ChainType> MonitorManager::detectChainType(const QString& address)
{
    return QCoro::Task<ChainType>();
}
}
