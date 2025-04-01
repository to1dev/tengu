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

#include <QObject>
#include <QString>
#include <QTimer>

#include "qcoro/QCoro"
#include "qcoro/QCoroCore"

#include "BlockchainProvider.h"
#include "ProviderFactory.h"

namespace Daitengu::Wallets {

class BlockchainMonitor : public QObject {
    Q_OBJECT

public:
    explicit BlockchainMonitor(ChainType chainType,
        ProviderType initialProvider = ProviderType::NONE,
        QObject* parent = nullptr);

    virtual ~BlockchainMonitor();

    virtual QCoro::Task<void> setAddress(const QString& address);

    [[nodiscard]] QString address() const
    {
        return currentAddress_;
    }

    virtual QCoro::Task<bool> connect();
    virtual QCoro::Task<void> disconnect();
    virtual QCoro::Task<void> refreshBalance();
    virtual QCoro::Task<void> refreshTokens();

    [[nodiscard]] virtual bool isConnected() const;
    [[nodiscard]] virtual bool isConnecting() const;
    [[nodiscard]] virtual QCoro::Task<bool> isValidAddress(
        const QString& address);

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

protected:
    QString currentAddress_;

private:
    ChainType chainType_;
    ProviderType currentProviderType_;
    std::unique_ptr<BlockchainProvider> provider_;

    std::unique_ptr<QTimer> autoRefreshTimer_;
    int autoRefreshInterval_ { 0 };

    QCoro::Task<void> setupProviderConnections();
    QString balanceResultToString(const BalanceResult& balance) const;

    std::unique_ptr<QTimer> connectionRetryTimer_;
    int connectionRetryCount_ { 0 };
    static const int MAX_CONNECTION_RETRIES { 5 };
};

}
