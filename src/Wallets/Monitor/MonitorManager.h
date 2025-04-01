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

#include <QMap>
#include <QObject>
#include <QRegularExpression>

#include "qcoro/QCoro"

#include "BlockchainMonitor.h"

namespace Daitengu::Wallets {

class MonitorManager : public QObject {
    Q_OBJECT

public:
    explicit MonitorManager(QObject* parent = nullptr);
    ~MonitorManager();

    QCoro::Task<void> setAddress(const QString& address);

    [[nodiscard]] QString currentAddress() const
    {
        return currentAddress_;
    }

    QCoro::Task<void> refreshBalance();
    QCoro::Task<void> refreshTokens();

    void registerMonitor(ChainType type, BlockchainMonitor* monitor);

    BlockchainMonitor* getMonitor(ChainType chainType);

    QCoro::Task<void> setAutoRefreshInterval(int milliseconds);

Q_SIGNALS:
    void balanceUpdated(
        const QString& address, const QString& balance, ChainType chain);
    void tokensUpdated(const QString& address, const QList<TokenInfo>& tokens,
        ChainType chain);
    void addressChanged(const QString& address, ChainType chain);
    void error(const QString& message, ChainType chain);

private Q_SLOTS:
    void onBalanceUpdated(const QString& address, const QString& balance);
    void onTokensUpdated(
        const QString& address, const QList<TokenInfo>& tokens);
    void onMonitorError(const QString& message);

private:
    [[nodiscard]] QCoro::Task<ChainType> detectChainType(
        const QString& address);

    QString currentAddress_;

    BlockchainMonitor* activeMonitor_ { nullptr };

    QMap<ChainType, BlockchainMonitor*> monitors_;

    ChainType currentChainType_ { ChainType::BITCOIN };

    int autoRefreshInterval_ { 0 };
};

}
