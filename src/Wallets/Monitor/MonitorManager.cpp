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

void BlockchainMonitorRegistry::registerMonitor(
    ChainType type, std::unique_ptr<BlockchainMonitor> monitor)
{
    if (monitors_.contains(type)) {
        spdlog::warn(
            "Monitor for chain type {} already registered, replacing it",
            static_cast<int>(type));
    }

    monitors_[type] = std::move(monitor);

    spdlog::info(
        "Registered monitor for chain type: {}", static_cast<int>(type));
}

BlockchainMonitor* BlockchainMonitorRegistry::getMonitor(ChainType type)
{
    auto it = monitors_.find(type);
    return (it != monitors_.end()) ? it->second.get() : nullptr;
}

MonitorManager::MonitorManager(QObject* parent)
    : QObject(parent)
{
    spdlog::info("MonitorManager initialized");
}

MonitorManager::~MonitorManager() = default;

QCoro::Task<void> MonitorManager::setAddress(const QString& address)
{
    if (currentAddress_ != address) {
        spdlog::info(
            "Setting new address to monitor: {}", address.toStdString());

        ChainType chainType = co_await detectChainType(address);

        if (chainType != currentChainType_ || !activeMonitor_) {
            activeMonitor_ = registry_.getMonitor(chainType);
            if (!activeMonitor_) {
                spdlog::warn("No monitor registered for chain type: {}",
                    static_cast<int>(chainType));
                Q_EMIT error(
                    "No monitor available for this address type", chainType);
                co_return;
            }
            currentChainType_ = chainType;
            spdlog::info(
                "Switched to chain type: {}", static_cast<int>(chainType));
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

    co_return;
}

QCoro::Task<void> MonitorManager::refreshBalance()
{
    if (activeMonitor_) {
        spdlog::debug("Manually refreshing balance for address: {}",
            currentAddress_.toStdString());
        co_await activeMonitor_->refreshBalance();
    }

    co_return;
}

QCoro::Task<void> MonitorManager::refreshTokens()
{
    if (activeMonitor_) {
        spdlog::debug("Manually refreshing tokens for address: {}",
            currentAddress_.toStdString());
        co_await activeMonitor_->refreshTokens();
    }

    co_return;
}

void MonitorManager::registerMonitor(
    ChainType type, std::unique_ptr<BlockchainMonitor> monitor)
{
    if (!monitor) {
        spdlog::warn("Attempting to register null monitor for chain type: {}",
            static_cast<int>(type));
        return;
    }

    registry_.registerMonitor(type, std::move(monitor));

    auto* regMonitor = registry_.getMonitor(type);

    connect(regMonitor, &BlockchainMonitor::balanceUpdated, this,
        &MonitorManager::onBalanceUpdated);
    connect(regMonitor, &BlockchainMonitor::tokensUpdated, this,
        &MonitorManager::onTokensUpdated);
    connect(regMonitor, &BlockchainMonitor::error, this,
        &MonitorManager::onMonitorError);

    regMonitor->setAutoRefreshInterval(autoRefreshInterval_);

    if (!activeMonitor_) {
        activeMonitor_ = regMonitor;
        currentChainType_ = type;
    }
}

BlockchainMonitor* MonitorManager::getMonitor(ChainType chainType)
{
    return registry_.getMonitor(chainType);
}

QCoro::Task<void> MonitorManager::setAutoRefreshInterval(int milliseconds)
{
    autoRefreshInterval_ = milliseconds;

    for (int type = static_cast<int>(ChainType::BITCOIN);
        type <= static_cast<int>(ChainType::SOLANA); ++type) {
        ChainType chainType = static_cast<ChainType>(type);
        if (auto* monitor = registry_.getMonitor(chainType)) {
            co_await monitor->setAutoRefreshInterval(milliseconds);
        }
    }

    spdlog::info(
        "Set auto-refresh interval to {}ms for all monitors", milliseconds);

    co_return;
}

void MonitorManager::onBalanceUpdated(
    const QString& address, const QString& balance)
{
    auto* monitor = qobject_cast<BlockchainMonitor*>(sender());
    if (monitor) {
        Q_EMIT balanceUpdated(address, balance, monitor->chainType());
        spdlog::debug(
            "Balance updated for address: {} on chain: {} - Value: {}",
            address.toStdString(), static_cast<int>(monitor->chainType()),
            balance.toStdString());
    }
}

void MonitorManager::onTokensUpdated(
    const QString& address, const QList<TokenInfo>& tokens)
{
    auto* monitor = qobject_cast<BlockchainMonitor*>(sender());
    if (monitor) {
        Q_EMIT tokensUpdated(address, tokens, monitor->chainType());
        spdlog::debug(
            "Tokens updated for address: {} on chain: {} - Token count: {}",
            address.toStdString(), static_cast<int>(monitor->chainType()),
            tokens.size());
    }
}

void MonitorManager::onMonitorError(const QString& message)
{
    auto* monitor = qobject_cast<BlockchainMonitor*>(sender());
    if (monitor) {
        Q_EMIT error(message, monitor->chainType());
        spdlog::warn("Error from chain {}: {}",
            static_cast<int>(monitor->chainType()), message.toStdString());
    }
}

QCoro::Task<ChainType> MonitorManager::detectChainType(const QString& address)
{
    for (int type = static_cast<int>(ChainType::BITCOIN);
        type <= static_cast<int>(ChainType::SOLANA); ++type) {
        ChainType chainType = static_cast<ChainType>(type);
        if (auto* monitor = registry_.getMonitor(chainType)) {
            if (co_await monitor->isValidAddress(address)) {
                spdlog::debug("Address {} detected as chain type: {}",
                    address.toStdString(),
                    static_cast<int>(monitor->chainType()));
                co_return monitor->chainType();
            }
        }
    }

    static const QRegularExpression ethRegex("^0x[a-fA-F0-9]{40}$");
    static const QRegularExpression btcLegacyRegex(
        "^[13][a-km-zA-HJ-NP-Z1-9]{25,34}$");
    static const QRegularExpression btcSegwitRegex("^bc1[a-zA-Z0-9]{25,90}$");

    if (ethRegex.match(address).hasMatch()) {
        spdlog::debug(
            "Address {} detected as Ethereum by regex", address.toStdString());
        co_return ChainType::ETHEREUM;
    } else if (btcLegacyRegex.match(address).hasMatch()
        || btcSegwitRegex.match(address).hasMatch()) {
        spdlog::debug(
            "Address {} detected as Bitcoin by regex", address.toStdString());
        co_return ChainType::BITCOIN;
    }

    spdlog::warn("Could not determine chain type for address: {} - defaulting "
                 "to Bitcoin",
        address.toStdString());

    co_return ChainType::BITCOIN;
}

}

/*
int main(int argc, char* argv[])
{
    MonitorManager manager;

    auto btcMonitor = std::make_unique<BlockchainMonitor>(
        ChainType::BITCOIN, ProviderType::MEMPOOL_SPACE);
    manager.registerMonitor(ChainType::BITCOIN, std::move(btcMonitor));

    QObject::connect(&manager, &MonitorManager::balanceUpdated,
        [](const QString& address, const QString& balance, ChainType chain) {
            spdlog::info("Balance updated for address {} on chain {}: {}",
                address.toStdString(), static_cast<int>(chain),
                balance.toStdString());
            qDebug() << "Address:" << address << "Balance:" << balance
                     << "Chain:" << static_cast<int>(chain);
        });

    QObject::connect(&manager, &MonitorManager::error,
        [](const QString& message, ChainType chain) {
            spdlog::error("Error on chain {}: {}", static_cast<int>(chain),
                message.toStdString());
            qDebug() << "Error:" << message
                     << "Chain:" << static_cast<int>(chain);
        });

    QString address = "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa";
    spdlog::info("Querying balance for address: {}", address.toStdString());

    manager.setAddress(address)
        .then([&manager]() { return manager.refreshBalance(); })
        .then([]() {
            spdlog::info("Balance query completed");
            QTimer::singleShot(
                2000, &QCoreApplication::instance(), &QCoreApplication::quit);
        });
}
*/
