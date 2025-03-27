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

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QWebSocket>
#include <QtConcurrent>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

#include "../BlockchainProvider.h"

namespace Daitengu::Wallets {

class MempoolSpaceBitcoinProvider : public BlockchainProvider {
    Q_OBJECT

public:
    explicit MempoolSpaceBitcoinProvider(QObject* parent = nullptr);
    ~MempoolSpaceBitcoinProvider() override;

    QFuture<ProviderResponse<BalanceResult>> getBalance(
        const QString& address) override;
    QFuture<ProviderResponse<TokenList>> getTokens(
        const QString& address) override;
    QFuture<ProviderResponse<bool>> isValidAddress(
        const QString& address) override;

    bool supportsRealTimeUpdates() const override
    {
        return true;
    }

    bool subscribeToAddressChanges(const QString& address) override;
    void unsubscribeFromAddressChanges(const QString& address) override;

    QString providerName() const override
    {
        return "mempool.space";
    }

    QString providerUrl() const override
    {
        return "https://mempool.space";
    }

    QUrl providerIconUrl() const override
    {
        return QUrl("https://mempool.space/resources/mempool-logo.png");
    }

    bool initialize() override;
    void shutdown() override;

private Q_SLOTS:
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketTextMessageReceived(const QString& message);
    void onWebSocketError(QAbstractSocket::SocketError error);
    void onSslErrors(const QList<QSslError>& errors);

private:
    std::unique_ptr<QWebSocket> webSocket_;
    QMap<QString, bool> subscribedAddresses_;
    bool connected_ { false };

    std::unique_ptr<QTimer> pingTimer_;
    QDateTime lastPongTime_;
    int consecutivePingFailures_ { 0 };

    static const QString API_BASE_URL;
    static const QString WS_BASE_URL;

    void setupPingPong();
    void checkPingPongHealth();
    ProviderResponse<BalanceResult> parseBalanceResponse(
        const nlohmann::json& response);
};
}
