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

    auto manager = SolanaConnectionManager::instance();
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

    auto manager = SolanaConnectionManager::instance();

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

void SmartMoneyTracker::addSmartMoneyAddress(std::string_view address)
{
    QString _address = QString::fromStdString(std::string(address));
    criteria_.smartAddresses.insert(_address);

    if (isTracking_ && !accountListenerIds_.contains(_address)) {
        auto manager = SolanaConnectionManager::instance();
        int listenerId = manager->registerAccountListener(
            address, [this](const json& data) { processAccountUpdate(data); });

        accountListenerIds_[_address] = listenerId;
    }
}

void SmartMoneyTracker::removeSmartMoneyAddress(std::string_view address)
{
    QString _address = QString::fromStdString(std::string(address));
    criteria_.smartAddresses.remove(_address);

    if (isTracking_ && accountListenerIds_.contains(_address)) {
        auto manager = SolanaConnectionManager::instance();
        manager->unregisterListener(accountListenerIds_[_address]);
        accountListenerIds_.remove(_address);
    }
}

void SmartMoneyTracker::addTrackedProgramId(std::string_view programId)
{
    criteria_.trackedProgramIds.insert(
        QString::fromStdString(std::string(programId)));
}

void SmartMoneyTracker::removeTrackedProgramId(std::string_view programId)
{
    criteria_.trackedProgramIds.remove(
        QString::fromStdString(std::string(programId)));
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

void SmartMoneyTracker::setName(std::string_view name)
{
    name_ = QString::fromStdString(std::string(name));
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
    bool involvesSmart = checkSmartAddresses(transaction);
    if (!involvesSmart) {
        return false;
    }

    if (!checkTransactionAmount(transaction)) {
        return false;
    }

    if (!criteria_.trackedProgramIds.isEmpty()
        && !checkProgramIds(transaction)) {
        return false;
    }

    if (criteria_.customFilter && !criteria_.customFilter(transaction)) {
        return false;
    }

    return true;
}

bool SmartMoneyTracker::checkSmartAddresses(const json& transaction) const
{
    if (criteria_.smartAddresses.isEmpty()) {
        return false;
    }

    if (!transaction.contains("transaction")) {
        return false;
    }

    const json& txDetails = transaction["transaction"];

    if (!txDetails.contains("message") || !txDetails["message"].is_object()) {
        return false;
    }

    const json& message = txDetails["message"];

    if (!message.contains("accountKeys")
        || !message["accountKeys"].is_array()) {
        return false;
    }

    const json& accounts = message["accountKeys"];

    for (const auto& account : accounts) {
        if (!account.is_string())
            continue;

        QString accountStr = QString::fromStdString(account.get<std::string>());
        if (criteria_.smartAddresses.contains(accountStr)) {
            return true;
        }
    }

    return false;
}

bool SmartMoneyTracker::checkProgramIds(const json& transaction) const
{
    if (criteria_.trackedProgramIds.isEmpty()) {
        return true;
    }

    if (!transaction.contains("transaction")) {
        return false;
    }

    const json& txDetails = transaction["transaction"];

    if (!txDetails.contains("message") || !txDetails["message"].is_object()) {
        return false;
    }

    const json& message = txDetails["message"];

    if (!message.contains("instructions")
        || !message["instructions"].is_array()) {
        return false;
    }

    const json& instructions = message["instructions"];

    for (const auto& instruction : instructions) {
        if (!instruction.is_object())
            continue;

        if (instruction.contains("programId")) {
            QString programId = QString::fromStdString(
                instruction["programId"].get<std::string>());

            if (criteria_.trackedProgramIds.contains(programId)) {
                return true;
            }
        }
    }

    return false;
}

bool SmartMoneyTracker::checkTransactionAmount(const json& transaction) const
{
    if (criteria_.minTransactionAmount == 0) {
        return true;
    }

    if (!transaction.contains("transaction")) {
        return false;
    }

    const json& txDetails = transaction["transaction"];

    if (!txDetails.contains("meta") || !txDetails["meta"].is_object()) {
        return false;
    }

    const json& meta = txDetails["meta"];

    if (!meta.contains("postBalances") || !meta.contains("preBalances")
        || !meta["postBalances"].is_array()
        || !meta["preBalances"].is_array()) {
        return false;
    }

    const json& preBalances = meta["preBalances"];
    const json& postBalances = meta["postBalances"];

    uint64_t maxChange = 0;
    size_t minSize = std::min(preBalances.size(), postBalances.size());

    for (size_t i = 0; i < minSize; i++) {
        if (!preBalances[i].is_number() || !postBalances[i].is_number()) {
            continue;
        }

        uint64_t pre = preBalances[i].get<uint64_t>();
        uint64_t post = postBalances[i].get<uint64_t>();
        uint64_t change = pre > post ? pre - post : post - pre;
        maxChange = std::max(maxChange, change);
    }

    return maxChange >= criteria_.minTransactionAmount;
}

void SmartMoneyTracker::reregisterListeners()
{
    stopTracking();
    startTracking();
}

void SmartMoneyTracker::registerAccountListeners()
{
    auto manager = SolanaConnectionManager::instance();

    for (const QString& address : criteria_.smartAddresses) {
        if (!accountListenerIds_.contains(address)) {
            int listenerId
                = manager->registerAccountListener(address.toStdString(),
                    [this](const json& data) { processAccountUpdate(data); });
            accountListenerIds_[address] = listenerId;
        }
    }
}
}
