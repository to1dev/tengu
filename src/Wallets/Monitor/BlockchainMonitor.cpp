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

BlockchainMonitor::BlockchainMonitor(
    ChainType chainType, ProviderType initialProvider, QObject* parent)
    : QObject(parent)
    , chainType_(chainType)
    , currentProviderType_(ProviderType::NONE)
    , autoRefreshTimer_(std::make_unique<QTimer>())
    , connectionRetryTimer_(std::make_unique<QTimer>())
{
    autoRefreshTimer_->setParent(this);
    connectionRetryTimer_->setParent(this);

    QObject::connect(autoRefreshTimer_.get(), &QTimer::timeout, this,
        &BlockchainMonitor::onAutoRefreshTimer);

    if (initialProvider == ProviderType::NONE) {
        initialProvider = ProviderFactory::getDefaultProvider(chainType);
    }

    auto providerPtr
        = ProviderFactory::createProvider(chainType, initialProvider, this);
    if (providerPtr) {
        provider_.reset(providerPtr);
        currentProviderType_ = initialProvider;
        setupProviderConnections();

        qCInfo(bcMonitor) << "Created monitor for chain "
                          << static_cast<int>(chainType) << " with provider "
                          << ProviderFactory::getProviderName(initialProvider);
    } else {
        qCWarning(bcMonitor) << "Failed to create provider of type: "
                             << static_cast<int>(initialProvider)
                             << " for chain " << static_cast<int>(chainType);
    }

    connectionRetryTimer_->setSingleShot(true);
    QObject::connect(
        connectionRetryTimer_.get(), &QTimer::timeout, this, [this]() {
            if (!isConnected() && !isConnecting()) {
                if (connectionRetryCount_ < MAX_CONNECTION_RETRIES) {
                    connectionRetryCount_++;
                    qCInfo(bcMonitor)
                        << "Retry connection attempt" << connectionRetryCount_
                        << "for chain" << static_cast<int>(chainType_);
                    connect();
                } else {
                    qCWarning(bcMonitor)
                        << "Maximum connection retries reached for chain"
                        << static_cast<int>(chainType_);
                    Q_EMIT error("Failed to connect after multiple attempts");
                }
            }
        });
}

BlockchainMonitor::~BlockchainMonitor()
{
    disconnect();
}

QCoro::Task<void> BlockchainMonitor::setAddress(const QString& address)
{
    if (currentAddress_ != address) {
        qCInfo(bcMonitor) << "Setting address to " << address << " for chain "
                          << static_cast<int>(chainType_);
        currentAddress_ = address;

        if (!address.isEmpty() && provider_) {
            if (isConnected() && provider_->supportsRealTimeUpdates()) {
                co_await provider_->subscribeToAddressChanges(address);
            }

            co_await refreshBalance();
            co_await refreshTokens();
        }
    }
}

QCoro::Task<bool> BlockchainMonitor::connect()
{
    if (!provider_) {
        qCWarning(bcMonitor)
            << "Cannot connect: no provider available for chain "
            << static_cast<int>(chainType_);
        co_return false;
    }

    if (isConnected()) {
        qCDebug(bcMonitor) << "Already connected for chain "
                           << static_cast<int>(chainType_);
        co_return true;
    }

    if (isConnecting()) {
        qCDebug(bcMonitor) << "Connection already in progress for chain "
                           << static_cast<int>(chainType_);
        co_return true;
    }

    bool result = co_await provider_->initialize();

    if (!result && !provider_->isConnected()) {
        connectionRetryCount_ = 0;
        connectionRetryTimer_->start(5000);
    }

    if (result && !currentAddress_.isEmpty()
        && provider_->supportsRealTimeUpdates()) {
        co_await provider_->subscribeToAddressChanges(currentAddress_);
    }

    if (autoRefreshInterval_ > 0) {
        autoRefreshTimer_->start(autoRefreshInterval_);
    }

    co_return result;
}

QCoro::Task<void> BlockchainMonitor::disconnect()
{
    if (!provider_) {
        co_return;
    }

    if (autoRefreshTimer_->isActive()) {
        autoRefreshTimer_->stop();
    }

    if (connectionRetryTimer_->isActive()) {
        connectionRetryTimer_->stop();
    }

    if (!currentAddress_.isEmpty() && provider_->supportsRealTimeUpdates()) {
        co_await provider_->unsubscribeFromAddressChanges(currentAddress_);
    }

    co_await provider_->shutdown();

    qCInfo(bcMonitor) << "Disconnected for chain "
                      << static_cast<int>(chainType_);
}

QCoro::Task<void> BlockchainMonitor::refreshBalance()
{
    if (!provider_ || currentAddress_.isEmpty()) {
        co_return;
    }

    qCDebug(bcMonitor) << "Refreshing balance for address " << currentAddress_
                       << " on chain " << static_cast<int>(chainType_);

    auto response = co_await provider_->getBalance(currentAddress_);

    if (response.success && response.data) {
        QString balanceStr = balanceResultToString(response.data.value());
        Q_EMIT balanceUpdated(currentAddress_, balanceStr);
    } else if (response.error) {
        qCWarning(bcMonitor)
            << "Balance refresh error: " << response.error->message;
        Q_EMIT error(response.error->message);
    }
}

QCoro::Task<void> BlockchainMonitor::refreshTokens()
{
    if (!provider_ || currentAddress_.isEmpty()) {
        co_return;
    }

    qCDebug(bcMonitor) << "Refreshing tokens for address " << currentAddress_
                       << " on chain " << static_cast<int>(chainType_);

    auto response = co_await provider_->getTokens(currentAddress_);

    if (response.success && response.data) {
        Q_EMIT tokensUpdated(currentAddress_, response.data.value());
    } else if (response.error) {
        qCWarning(bcMonitor)
            << "Tokens refresh error: " << response.error->message;
        Q_EMIT error(response.error->message);
    }
}

bool BlockchainMonitor::isConnected() const
{
    return provider_ && provider_->isConnected();
}

bool BlockchainMonitor::isConnecting() const
{
    return provider_ && provider_->isConnecting();
}

QCoro::Task<bool> BlockchainMonitor::isValidAddress(const QString& address)
{
    if (!provider_) {
        co_return false;
    }

    auto response = co_await provider_->isValidAddress(address);

    if (response.success && response.data) {
        co_return response.data.value();
    }

    co_return false;
}

QCoro::Task<bool> BlockchainMonitor::switchProvider(ProviderType newProvider)
{
    if (currentProviderType_ == newProvider || !provider_) {
        co_return true;
    }

    qCInfo(bcMonitor) << "Switching provider from "
                      << ProviderFactory::getProviderName(currentProviderType_)
                      << " to " << ProviderFactory::getProviderName(newProvider)
                      << " for chain " << static_cast<int>(chainType_);

    QString currentAddr = currentAddress_;

    if (autoRefreshTimer_->isActive()) {
        autoRefreshTimer_->stop();
    }

    if (!currentAddr.isEmpty() && provider_->supportsRealTimeUpdates()) {
        co_await provider_->unsubscribeFromAddressChanges(currentAddr);
    }

    co_await provider_->shutdown();

    auto newProviderInstance
        = ProviderFactory::createProvider(chainType_, newProvider, this);
    if (!newProviderInstance) {
        qCWarning(bcMonitor) << "Failed to create new provider of type: "
                             << static_cast<int>(newProvider);
        co_return false;
    }

    provider_.reset(newProviderInstance);
    currentProviderType_ = newProvider;

    co_await setupProviderConnections();

    bool initResult = co_await provider_->initialize();

    if (!currentAddr.isEmpty()) {
        auto validResponse = co_await provider_->isValidAddress(currentAddr);
        bool isValid
            = validResponse.success && validResponse.data.value_or(false);

        if (isValid) {
            co_await setAddress(currentAddr);
        } else {
            qCWarning(bcMonitor)
                << "Address format incompatible with new provider: "
                << currentAddr;
            Q_EMIT error("Address format incompatible with new provider");
        }
    }

    if (autoRefreshInterval_ > 0) {
        autoRefreshTimer_->start(autoRefreshInterval_);
    }

    co_return initResult;
}

QList<ProviderType> BlockchainMonitor::availableProviders() const
{
    return ProviderFactory::getAvailableProviders(chainType_);
}

QCoro::Task<void> BlockchainMonitor::setAutoRefreshInterval(int milliseconds)
{
    autoRefreshInterval_ = milliseconds;

    if (autoRefreshTimer_->isActive()) {
        autoRefreshTimer_->stop();
    }

    if (milliseconds > 0 && isConnected()) {
        autoRefreshTimer_->start(milliseconds);
        qCDebug(bcMonitor) << "Auto-refresh set to " << milliseconds
                           << "ms for chain " << static_cast<int>(chainType_);
    } else {
        qCDebug(bcMonitor) << "Auto-refresh disabled for chain "
                           << static_cast<int>(chainType_);
    }

    co_return;
}

void BlockchainMonitor::onProviderBalanceChanged(
    const QString& address, const BalanceResult& balance)
{
    QString balanceStr = balanceResultToString(balance);
    Q_EMIT balanceUpdated(address, balanceStr);
}

void BlockchainMonitor::onProviderTokensChanged(
    const QString& address, const TokenList& tokens)
{
    Q_EMIT tokensUpdated(address, tokens);
}

void BlockchainMonitor::onProviderConnectionChanged(bool connected)
{
    if (connected) {
        connectionRetryTimer_->stop();
        connectionRetryCount_ = 0;

        if (!currentAddress_.isEmpty()
            && provider_->supportsRealTimeUpdates()) {
            provider_->subscribeToAddressChanges(currentAddress_)
                .then([](bool) { });
        }

        if (autoRefreshInterval_ > 0 && !autoRefreshTimer_->isActive()) {
            autoRefreshTimer_->start(autoRefreshInterval_);
        }
    } else {
        if (autoRefreshTimer_->isActive()) {
            autoRefreshTimer_->stop();
        }
    }

    Q_EMIT connectionStatusChanged(connected);
}

void BlockchainMonitor::onProviderError(const QString& message)
{
    Q_EMIT error(message);
}

void BlockchainMonitor::onAutoRefreshTimer()
{
    if (isConnected() && !currentAddress_.isEmpty()) {
        refreshBalance();
        refreshTokens();
    }
}

QCoro::Task<void> BlockchainMonitor::setupProviderConnections()
{
    if (!provider_) {
        co_return;
    }

    QObject::connect(provider_.get(), &BlockchainProvider::balanceChanged, this,
        &BlockchainMonitor::onProviderBalanceChanged);
    QObject::connect(provider_.get(), &BlockchainProvider::tokensChanged, this,
        &BlockchainMonitor::onProviderTokensChanged);
    QObject::connect(provider_.get(),
        &BlockchainProvider::connectionStatusChanged, this,
        &BlockchainMonitor::onProviderConnectionChanged);
    QObject::connect(provider_.get(), &BlockchainProvider::error, this,
        &BlockchainMonitor::onProviderError);

    co_return;
}

QString BlockchainMonitor::balanceResultToString(
    const BalanceResult& balance) const
{
    QString balanceStr;
    std::visit(
        [&balanceStr](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, QString>) {
                balanceStr = arg;
            } else if constexpr (std::is_same_v<T, std::string>) {
                balanceStr = QString::fromStdString(arg);
            } else if constexpr (std::is_same_v<T, double>) {
                balanceStr = QString::number(arg, 'f', 8);
            }
        },
        balance);

    return balanceStr;
}
}
