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

#include <memory>
#include <optional>

#include <QCache>
#include <QObject>
#include <QString>
#include <QTimer>

#include <spdlog/spdlog.h>

#include "qcoro/QCoro"
#include "qcoro/QCoroCore"

#include "BlockchainProvider.h"
#include "ProviderFactory.h"

namespace Daitengu::Wallets {

/**
 * @brief Monitors a specific blockchain address for balance and token updates.
 */
class BlockchainMonitor : public QObject {
    Q_OBJECT

public:
    explicit BlockchainMonitor(ChainType chainType,
        ProviderType initialProvider = ProviderType::NONE,
        QObject* parent = nullptr);

    ~BlockchainMonitor() override;

    QCoro::Task<void> setAddress(const QString& address);

    [[nodiscard]] QString address() const
    {
        return currentAddress_;
    }

    QCoro::Task<bool> connect();
    QCoro::Task<void> disconnect();
    QCoro::Task<void> refreshBalance();
    QCoro::Task<void> refreshTokens();

    [[nodiscard]] bool isConnected() const;
    [[nodiscard]] bool isConnecting() const;
    [[nodiscard]] QCoro::Task<bool> isValidAddress(const QString& address);

    QCoro::Task<bool> switchProvider(ProviderType newProvider);

    ProviderType currentProvider() const
    {
        return currentProviderType_;
    }

    ChainType chainType() const
    {
        return chainType_;
    }

    QList<ProviderType> availableProviders() const;

    QCoro::Task<void> setAutoRefreshInterval(int milliseconds);

Q_SIGNALS:
    void balanceUpdated(const QString& address, const QString& balance);
    void tokensUpdated(const QString& address, const QList<TokenInfo>& tokens);
    void connectionStatusChanged(bool connected);
    void error(const QString& message);

private Q_SLOTS:
    void onProviderBalanceChanged(
        const QString& address, const BalanceResult& balance);
    void onProviderTokensChanged(
        const QString& address, const TokenList& tokens);
    void onProviderConnectionChanged(bool connected);
    void onProviderError(const QString& message);
    void onAutoRefreshTimer();

private:
    QCoro::Task<void> setupProviderConnections();
    QString balanceResultToString(const BalanceResult& balance) const;

    ChainType chainType_;
    ProviderType currentProviderType_;
    std::unique_ptr<BlockchainProvider> provider_;

    QString currentAddress_;

    std::unique_ptr<QTimer> autoRefreshTimer_;
    int autoRefreshInterval_ { 0 };

    std::unique_ptr<QTimer> connectionRetryTimer_;
    int connectionRetryCount_ { 0 };
    QCache<QString, QString> balanceCache_ { 100 };
    static constexpr int MAX_CONNECTION_RETRIES { 5 };
};

}
