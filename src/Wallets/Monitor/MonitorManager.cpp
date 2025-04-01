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
    : QObject(parent)
{
    qCInfo(bcMonitor) << "MonitorManager initialized";
}

MonitorManager::~MonitorManager() = default;

QCoro::Task<void> MonitorManager::setAddress(const QString& address)
{
    if (currentAddress_ != address) {
        qCInfo(bcMonitor) << "Setting new address to monitor: " << address;

        ChainType chainType = co_await detectChainType(address);

        if (chainType != currentChainType_ || !activeMonitor_) {
            if (activeMonitor_ && activeMonitor_->isConnected()) {
                co_await activeMonitor_->disconnect();
            }

            auto it = monitors_.find(chainType);
            if (it != monitors_.end()) {
                activeMonitor_ = it.value();
                currentChainType_ = chainType;
                qCInfo(bcMonitor) << "Switched to chain type: "
                                  << static_cast<int>(chainType);
            } else {
                qCWarning(bcMonitor) << "No monitor registered for chain type: "
                                     << static_cast<int>(chainType);
                Q_EMIT error(
                    "No monitor available for this address type ", chainType);
                co_return;
            }
        }

        currentAddress_ = address;

        if (activeMonitor_) {
            co_await activeMonitor_->setAddress(address);
            if (!activeMonitor_->isConnected()
                && !activeMonitor_->isConnecting()) {
                co_await activeMonitor_->connect();
            }
        }

        Q_EMIT addressChanged(address, currentChainType_);
    }
}

QCoro::Task<void> MonitorManager::refreshBalance()
{
    if (activeMonitor_) {
        qCDebug(bcMonitor) << "Manually refreshing balance for address: "
                           << currentAddress_;
        co_await activeMonitor_->refreshBalance();
    }
}

QCoro::Task<void> MonitorManager::refreshTokens()
{
    if (activeMonitor_) {
        qCDebug(bcMonitor) << "Manually refreshing tokens for address: "
                           << currentAddress_;
        co_await activeMonitor_->refreshTokens();
    }
}

void MonitorManager::registerMonitor(ChainType type, BlockchainMonitor* monitor)
{
    if (!monitor) {
        qCWarning(bcMonitor)
            << "Attempting to register null monitor for chain type: "
            << static_cast<int>(type);
        return;
    }

    monitors_[type] = monitor;

    connect(monitor, &BlockchainMonitor::balanceUpdated, this,
        &MonitorManager::onBalanceUpdated);
    connect(monitor, &BlockchainMonitor::tokensUpdated, this,
        &MonitorManager::onTokensUpdated);
    connect(monitor, &BlockchainMonitor::error, this,
        &MonitorManager::onMonitorError);

    monitor->setAutoRefreshInterval(autoRefreshInterval_);

    if (!activeMonitor_) {
        activeMonitor_ = monitor;
        currentChainType_ = type;
    }

    qCInfo(bcMonitor) << "Registered monitor for chain type: "
                      << static_cast<int>(type);
}

BlockchainMonitor* MonitorManager::getMonitor(ChainType chainType)
{
    auto it = monitors_.find(chainType);
    if (it != monitors_.end()) {
        return it.value();
    }

    return nullptr;
}

QCoro::Task<void> MonitorManager::setAutoRefreshInterval(int milliseconds)
{
    autoRefreshInterval_ = milliseconds;

    for (auto it = monitors_.begin(); it != monitors_.end(); ++it) {
        co_await it.value()->setAutoRefreshInterval(milliseconds);
    }

    qCInfo(bcMonitor) << "Set auto-refresh interval to " << milliseconds
                      << "ms for all monitors";
}

void MonitorManager::onBalanceUpdated(
    const QString& address, const QString& balance)
{
    auto* monitor = qobject_cast<BlockchainMonitor*>(sender());
    if (monitor) {
        Q_EMIT balanceUpdated(address, balance, monitor->chainType());
        qCDebug(bcMonitor) << "Balance updated for address: " << address
                           << " on chain: "
                           << static_cast<int>(monitor->chainType())
                           << " - Value: " << balance;
    }
}

void MonitorManager::onTokensUpdated(
    const QString& address, const QList<TokenInfo>& tokens)
{
    auto* monitor = qobject_cast<BlockchainMonitor*>(sender());
    if (monitor) {
        Q_EMIT tokensUpdated(address, tokens, monitor->chainType());
        qCDebug(bcMonitor) << "Tokens updated for address: " << address
                           << " on chain: "
                           << static_cast<int>(monitor->chainType())
                           << " - Token count: " << tokens.size();
    }
}

void MonitorManager::onMonitorError(const QString& message)
{
    auto* monitor = qobject_cast<BlockchainMonitor*>(sender());
    if (monitor) {
        Q_EMIT error(message, monitor->chainType());
        qCWarning(bcMonitor)
            << "Error from chain " << static_cast<int>(monitor->chainType())
            << " : " << message;
    }
}

QCoro::Task<ChainType> MonitorManager::detectChainType(const QString& address)
{
    for (auto it = monitors_.constBegin(); it != monitors_.constEnd(); ++it) {
        if (it.value()) {
            bool isValid = co_await it.value()->isValidAddress(address);
            if (isValid) {
                qCDebug(bcMonitor)
                    << "Address " << address
                    << "detected as chain type: " << static_cast<int>(it.key());
                co_return it.key();
            }
        }
    }

    static const QRegularExpression ethRegex("^0x[a-fA-F0-9]{40}$");
    static const QRegularExpression btcLegacyRegex(
        "^[13][a-km-zA-HJ-NP-Z1-9]{25,34}$");
    static const QRegularExpression btcSegwitRegex("^bc1[a-zA-Z0-9]{25,90}$");
    static const QRegularExpression solRegex("^[1-9A-HJ-NP-Za-km-z]{43,44}$");

    if (ethRegex.match(address).hasMatch()) {
        qCDebug(bcMonitor) << "Address " << address
                           << " detected as Ethereum by regex";
        co_return Wallets::ChainType::ETHEREUM;
    } else if (btcLegacyRegex.match(address).hasMatch()
        || btcSegwitRegex.match(address).hasMatch()) {
        qCDebug(bcMonitor) << "Address " << address
                           << " detected as Bitcoin by regex";
        co_return Wallets::ChainType::BITCOIN;
    } else if (solRegex.match(address).hasMatch()) {
        qCDebug(bcMonitor) << "Address " << address
                           << " detected as Solana by regex";
        co_return Wallets::ChainType::SOLANA;
    }

    qCWarning(bcMonitor) << "Could not determine chain type for address: "
                         << address << " - defaulting to Bitcoin";

    co_return Wallets::ChainType::BITCOIN;
}
}
