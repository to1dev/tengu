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
#include <QNetworkReply>
#include <QRegularExpression>
#include <QWebSocket>

#include "BlockchainMonitor.h"

namespace Daitengu::Wallets {

class BitcoinMonitor : public BlockchainMonitor {
    Q_OBJECT

public:
    explicit BitcoinMonitor(
        const QString& rpcUrl, const QString& wsUrl, QObject* parent = nullptr);
    ~BitcoinMonitor() override;

    bool connect() override;
    void disconnect() override;
    void refreshBalance() override;
    void refreshTokens() override;
    [[nodiscard]] bool isValidAddress(const QString& address) const override;

private Q_SLOTS:
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketTextMessageReceived(const QString& message);
    void onNetworkReplyFinished();

    void onWebSocketError(QAbstractSocket::SocketError error);
    void onSslErrors(const QList<QSslError>& errors);
    void onNetworkError(QNetworkReply::NetworkError error);

protected:
    void setupConnection() override;

private:
    QString rpcUrl_;
    QString wsUrl_;
    std::unique_ptr<QWebSocket> webSocket_;
    std::unique_ptr<QNetworkAccessManager> networkManager_;
    std::unique_ptr<QTimer> reconnectTimer_;

    std::unique_ptr<QTimer> pingTimer_;
    QDateTime lastPingTime_;
    QDateTime lastPongTime_;
    int pingLatency_ { 0 };
    int consecutivePingFailures_ { 0 };

    int requestId_ { 0 };

    void subscribeToAddress();

    void checkPingPongHealth();
    void setupPingPong();

    void sendJsonRpcRequest(const QString& method, const json& params);

    void processBalanceResponse(const json& response);
    void processTokenResponse(const json& response);

    [[nodiscard]] static bool isValidBitcoinAddress(const QString& address);
};

}
