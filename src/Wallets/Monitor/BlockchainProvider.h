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

#include <QNetworkAccessManager>
#include <QObject>

#include "qcoro/QCoro"
#include "qcoro/QCoroTask"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

#include "BlockchainTypes.h"

namespace Daitengu::Wallets {

class BlockchainProvider : public QObject {
    Q_OBJECT

public:
    explicit BlockchainProvider(QObject* parent = nullptr);
    virtual ~BlockchainProvider();

    virtual QCoro::Task<ProviderResponse<BalanceResult>> getBalance(
        const QString& address)
        = 0;
    virtual QCoro::Task<ProviderResponse<TokenList>> getTokens(
        const QString& address)
        = 0;
    virtual QCoro::Task<ProviderResponse<bool>> isValidAddress(
        const QString& address)
        = 0;

    virtual bool supportsRealTimeUpdates() const = 0;
    virtual QCoro::Task<bool> subscribeToAddressChanges(const QString& address)
        = 0;
    virtual QCoro::Task<void> unsubscribeFromAddressChanges(
        const QString& address)
        = 0;

    virtual QString providerName() const = 0;
    virtual QString providerUrl() const = 0;
    virtual QUrl providerIconUrl() const = 0;

    virtual bool isConnected() const
    {
        return connected_;
    }

    virtual bool isConnecting() const
    {
        return connecting_;
    }

    virtual QCoro::Task<bool> initialize() = 0;
    virtual QCoro::Task<void> shutdown() = 0;

Q_SIGNALS:
    void balanceChanged(
        const QString& address, const BalanceResult& newBalance);
    void tokensChanged(const QString& address, const TokenList& newTokens);
    void connectionStatusChanged(bool connected);
    void error(const QString& message);

protected:
    std::unique_ptr<QNetworkAccessManager> networkManager_;
    bool connected_ = false;
    bool connecting_ = false;
};

}
