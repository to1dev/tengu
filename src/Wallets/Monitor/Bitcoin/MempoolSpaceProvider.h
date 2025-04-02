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

#include <atomic>
#include <chrono>
#include <coroutine>

#include <QDateTime>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QSslError>
#include <QTimer>
#include <QWebSocket>

#include <spdlog/spdlog.h>

#include "qcoro/QCoroNetwork"
#include "qcoro/QCoroSignal"
#include "qcoro/QCoroTimer"
#include "qcoro/QCoroWebSocket"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

#include "../BlockchainProvider.h"

#include "../../Core/BitcoinWallet.h"

namespace Daitengu::Wallets {

class MempoolSpaceProvider : public BlockchainProvider {
    Q_OBJECT

public:
    explicit MempoolSpaceProvider(QObject* parent = nullptr);
    ~MempoolSpaceProvider() override;

    QCoro::Task<ProviderResponse<BalanceResult>> getBalance(
        const QString& address) override;
    QCoro::Task<ProviderResponse<TokenList>> getTokens(
        const QString& address) override;
    QCoro::Task<ProviderResponse<bool>> isValidAddress(
        const QString& address) override;

    bool supportsRealTimeUpdates() const override
    {
        return true;
    }

    QCoro::Task<bool> subscribeToAddressChanges(
        const QString& address) override;
    QCoro::Task<void> unsubscribeFromAddressChanges(
        const QString& address) override;

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
        return QUrl("https://mempool.space/resources/mempool-tube.png");
    }

    QCoro::Task<bool> initialize() override;
    QCoro::Task<void> shutdown() override;

private Q_SLOTS:
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketTextMessageReceived(const QString& message);
    void onWebSocketError(QAbstractSocket::SocketError socketError);
    void onSslErrors(const QList<QSslError>& errors);

private:
    std::unique_ptr<QWebSocket> webSocket_;
    QMap<QString, bool> subscribedAddresses_;
    std::atomic<bool> shutdownRequested_ { false };

    std::unique_ptr<QTimer> connectTimeoutTimer_;

    std::unique_ptr<QTimer> pingTimer_;
    QDateTime lastPongTime_;
    int consecutivePingFailures_ { 0 };

    static const QString API_BASE_URL;
    static const QString WS_BASE_URL;
    static constexpr int CONNECT_TIMEOUT_MS = 10000;
    static constexpr int PING_INTERVAL_MS = 30000;
    static constexpr int PING_TIMEOUT_MS = 90000;

    void setupPingPong();
    void checkPingPongHealth();
    QCoro::Task<void> waitForConnection(int timeoutMs = 10000);
    QCoro::Task<ProviderResponse<BalanceResult>> fetchBalanceFromAPI(
        const QString& address);
    ProviderResponse<BalanceResult> parseBalanceResponse(const json& response);

    QCoro::Task<QString> waitForMessage(int timeoutMs = 10000);

    static bool isValidBitcoinAddress(const QString& address);
};
}
