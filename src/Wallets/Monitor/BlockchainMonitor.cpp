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

#include "BlockchainMonitor.h"

namespace Daitengu::Wallets {

BlockchainMonitor::BlockchainMonitor(QObject* parent)
    : QObject(parent)
    , refreshTimer_(std::make_unique<QTimer>(this))
{
    QObject::connect(refreshTimer_.get(), &QTimer::timeout, this,
        &BlockchainMonitor::refreshBalance);
}

BlockchainMonitor::~BlockchainMonitor() = default;

void BlockchainMonitor::setAddress(const QString& address)
{
    if (currentAddress_ != address) {
        currentAddress_ = address;
        if (!currentAddress_.isEmpty() && isConnected()) {
            refreshBalance();
            refreshTokens();
        }
    }
}

void BlockchainMonitor::startRefreshTimer()
{
    if (refreshInterval_ > 0 && !currentAddress_.isEmpty() && isConnected()) {
        refreshTimer_->start(refreshInterval_);
    }
}

void BlockchainMonitor::setRefreshInterval(int milliseconds)
{
    refreshInterval_ = milliseconds;
    if (refreshTimer_->isActive()) {
        refreshTimer_->stop();
        startRefreshTimer();
    }
}
}
