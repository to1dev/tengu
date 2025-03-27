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
#include <variant>

#include <QFuture>
#include <QList>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QUrl>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

#include "BlockchainTypes.h"

namespace Daitengu::Wallets {

using BalanceResult = std::variant<QString, std::string, double>;
using TokenList = QList<TokenInfo>;

using ProviderError = struct {
    int code;
    QString message;
};

template <typename T> struct ProviderResponse {
    bool success;
    std::optional<T> data;
    std::optional<ProviderError> error;
};

class BlockchainProvider : public QObject {
    Q_OBJECT

public:
    explicit BlockchainProvider(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    virtual ~BlockchainProvider() = default;

    virtual QFuture<ProviderResponse<BalanceResult>> getBalance(
        const QString& address)
        = 0;
    virtual QFuture<ProviderResponse<TokenList>> getTokens(
        const QString& address)
        = 0;
    virtual QFuture<ProviderResponse<bool>> isValidAddress(
        const QString& address)
        = 0;

    virtual bool supportsRealTimeUpdates() const = 0;
    virtual bool subscribeToAddressChanges(const QString& address) = 0;
    virtual void unsubscribeFromAddressChanges(const QString& address) = 0;

    virtual QString providerName() const = 0;
    virtual QString providerUrl() const = 0;
    virtual QUrl providerIconUrl() const = 0;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

Q_SIGNALS:
    void balanceChanged(
        const QString& address, const BalanceResult& newBalance);
    void tokensChanged(const QString& address, const TokenList& newTokens);
    void connectionStatusChanged(bool connected);
    void error(const QString& message);

protected:
    QNetworkAccessManager* networkManager_;
};

}
