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

#include "SmartMoneyTracker.h"

namespace Daitengu::Clients::Solana {

SmartMoneyTracker::SmartMoneyTracker(QObject* parent)
    : QObject(parent)
{
}

SmartMoneyTracker::~SmartMoneyTracker()
{
    stopTracking();
}

bool SmartMoneyTracker::startTracking()
{
    if (isTracking_) {
        return true;
    }

    auto* manager = SolanaConnectionManager::instance();
    if (!manager->isConnected()) {
        Q_EMIT error("Connection manager is not connected");
        return false;
    }

    transactionListenerId_ = manager->registerTransactionListener(
        [this](const json& transaction) { processTransaction(transaction); });

    registerAccountListeners();

    isTracking_ = true;
    Q_EMIT trackingStatusChanged(true);

    qDebug() << name_ << " started tracking";

    return true;
}

bool SmartMoneyTracker::stopTracking()
{
    return false;
}

void SmartMoneyTracker::setSmartMoneyCriteria(
    const SmartMoneyCriteria& criteria)
{
}

void SmartMoneyTracker::addSmartMoneyAddress(const QString& address)
{
}

void SmartMoneyTracker::removeSmartMoneyAddress(const QString& address)
{
}

void SmartMoneyTracker::addTrackedProgramId(const QString& programId)
{
}

void SmartMoneyTracker::removeTrackedProgramId(const QString& programId)
{
}

void SmartMoneyTracker::setMinTransactionAmount(uint64_t amount)
{
}

const SmartMoneyTracker::SmartMoneyCriteria&
SmartMoneyTracker::getCurrentCriteria() const
{
    return criteria_;
}

bool SmartMoneyTracker::isTracking() const
{
    return isTracking_;
}

void SmartMoneyTracker::setName(const QString& name)
{
    name_ = name;
}

QString SmartMoneyTracker::getName() const
{
    return name_;
}

void SmartMoneyTracker::processTransaction(const json& transaction)
{
}

void SmartMoneyTracker::processAccountUpdate(const json& accountData)
{
}

bool SmartMoneyTracker::isSmartMoneyTransaction(const json& transaction)
{
    return false;
}

void SmartMoneyTracker::reregisterListeners()
{
}

void SmartMoneyTracker::registerAccountListeners()
{
}
}
