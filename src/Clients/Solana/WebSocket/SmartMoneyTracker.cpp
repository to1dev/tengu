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

void SmartMoneyTracker::stopTracking()
{
    if (!isTracking_) {
        return;
    }

    auto* manager = SolanaConnectionManager::instance();

    if (transactionListenerId_ != -1) {
        manager->unregisterListener(transactionListenerId_);
        transactionListenerId_ = -1;
    }

    for (auto it = accountListenerIds_.begin(); it != accountListenerIds_.end();
        ++it) {
        manager->unregisterListener(it.value());
    }
    accountListenerIds_.clear();

    isTracking_ = false;
    Q_EMIT trackingStatusChanged(false);

    qDebug() << name_ << " stopped tracking";
}

void SmartMoneyTracker::setSmartMoneyCriteria(
    const SmartMoneyCriteria& criteria)
{
    criteria_ = criteria;

    if (isTracking_) {
        reregisterListeners();
    }
}

void SmartMoneyTracker::addSmartMoneyAddress(const QString& address)
{
    criteria_.smartAddresses.insert(address);

    if (isTracking_ && !accountListenerIds_.contains(address)) {
        auto* manager = SolanaConnectionManager::instance();
        int listenerId = manager->registerAccountListener(
            address, [this](const json& data) { processAccountUpdate(data); });

        accountListenerIds_[address] = listenerId;
    }
}

void SmartMoneyTracker::removeSmartMoneyAddress(const QString& address)
{
    criteria_.smartAddresses.remove(address);

    if (isTracking_ && accountListenerIds_.contains(address)) {
        auto* manager = SolanaConnectionManager::instance();
        manager->unregisterListener(accountListenerIds_[address]);
        accountListenerIds_.remove(address);
    }
}

void SmartMoneyTracker::addTrackedProgramId(const QString& programId)
{
    criteria_.trackedProgramIds.insert(programId);
}

void SmartMoneyTracker::removeTrackedProgramId(const QString& programId)
{
    criteria_.trackedProgramIds.remove(programId);
}

void SmartMoneyTracker::setMinTransactionAmount(uint64_t amount)
{
    criteria_.minTransactionAmount = amount;
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
    if (isSmartMoneyTransaction(transaction)) {
        Q_EMIT smartMoneyTransactionDetected(transaction);
    }
}

void SmartMoneyTracker::processAccountUpdate(const json& accountData)
{
    qDebug() << name_ << " Account update received for smart money wallet";
}

bool SmartMoneyTracker::isSmartMoneyTransaction(const json& transaction)
{
    return false;
}

void SmartMoneyTracker::reregisterListeners()
{
    stopTracking();
    startTracking();
}

void SmartMoneyTracker::registerAccountListeners()
{
    auto* manager = SolanaConnectionManager::instance();

    for (const QString& address : criteria_.smartAddresses) {
        if (!accountListenerIds_.contains(address)) {
            int listenerId = manager->registerAccountListener(address,
                [this](const json& data) { processAccountUpdate(data); });
            accountListenerIds_[address] = listenerId;
        }
    }
}
}
