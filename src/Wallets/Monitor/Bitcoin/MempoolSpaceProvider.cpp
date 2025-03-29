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

#include "MempoolSpaceProvider.h"

namespace Daitengu::Wallets {

const QString MempoolSpaceProvider::API_BASE_URL = "https://mempool.space/api";
const QString MempoolSpaceProvider::WS_BASE_URL = "wss://mempool.space/api/ws";

MempoolSpaceProvider::MempoolSpaceProvider(QObject* parent)
    : BlockchainProvider(parent)
    , webSocket_(std::make_unique<QWebSocket>())
    , connectTimeoutTimer_(std::make_unique<QTimer>())
    , pingTimer_(std::make_unique<QTimer>())
{
    webSocket_->setParent(this);
    connectTimeoutTimer_->setParent(this);
    pingTimer_->setParent(this);

    connectTimeoutTimer_->setSingleShot(true);
    connectTimeoutTimer_->setInterval(CONNECT_TIMEOUT_MS);

    connect(webSocket_.get(), &QWebSocket::connected, this,
        &MempoolSpaceProvider::onWebSocketConnected);
    connect(webSocket_.get(), &QWebSocket::disconnected, this,
        &MempoolSpaceProvider::onWebSocketDisconnected);
    connect(webSocket_.get(), &QWebSocket::textMessageReceived, this,
        &MempoolSpaceProvider::onWebSocketTextMessageReceived);
    connect(webSocket_.get(),
        QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this,
        &MempoolSpaceProvider::onWebSocketError);
    connect(webSocket_.get(), &QWebSocket::sslErrors, this,
        &MempoolSpaceProvider::onSslErrors);

    connect(connectTimeoutTimer_.get(), &QTimer::timeout, this, [this]() {
        if (connecting_ && !connected_) {
            qCWarning(bcMonitor)
                << "WebSocket connection timeout for" << providerName();
            connecting_ = false;
            webSocket_->abort();
            Q_EMIT error("Connection timeout");
            Q_EMIT connectionStatusChanged(false);
        }
    });
}

MempoolSpaceProvider::~MempoolSpaceProvider()
{
    shutdownRequested_ = true;
    shutdown();
}

QCoro::Task<ProviderResponse<BalanceResult>> MempoolSpaceProvider::getBalance(
    const QString& address)
{
    return QCoro::Task<ProviderResponse<BalanceResult>>();
}

QCoro::Task<ProviderResponse<TokenList>> MempoolSpaceProvider::getTokens(
    const QString& address)
{
    return QCoro::Task<ProviderResponse<TokenList>>();
}

QCoro::Task<ProviderResponse<bool>> MempoolSpaceProvider::isValidAddress(
    const QString& address)
{
    return QCoro::Task<ProviderResponse<bool>>();
}

QCoro::Task<bool> MempoolSpaceProvider::subscribeToAddressChanges(
    const QString& address)
{
    return QCoro::Task<bool>();
}

QCoro::Task<void> MempoolSpaceProvider::unsubscribeFromAddressChanges(
    const QString& address)
{
    return QCoro::Task<void>();
}

QCoro::Task<bool> MempoolSpaceProvider::initialize()
{
    shutdownRequested_ = false;

    if (connected_) {
        qCDebug(bcMonitor) << "Already connected to" << providerName();
        co_return true;
    }

    if (connecting_) {
        co_await waitForConnection();
        co_return connected_;
    }

    qCDebug(bcMonitor) << "Connecting to" << providerName() << "WebSocket";
    connecting_ = true;

    connectTimeoutTimer_->start();

    co_await qCoro(*webSocket_.get()).open(QUrl(WS_BASE_URL));

    if (shutdownRequested_) {
        co_return false;
    }

    if (!connected_) {
        onWebSocketConnected();
    }

    co_return connected_;
}

QCoro::Task<void> MempoolSpaceProvider::shutdown()
{
    return QCoro::Task<void>();
}

void MempoolSpaceProvider::onWebSocketConnected()
{
}

void MempoolSpaceProvider::onWebSocketDisconnected()
{
}

void MempoolSpaceProvider::onWebSocketTextMessageReceived(
    const QString& message)
{
}

void MempoolSpaceProvider::onWebSocketError(QAbstractSocket::SocketError error)
{
}

void MempoolSpaceProvider::onSslErrors(const QList<QSslError>& errors)
{
}

void MempoolSpaceProvider::setupPingPong()
{
}

void MempoolSpaceProvider::checkPingPongHealth()
{
}

QCoro::Task<void> MempoolSpaceProvider::waitForConnection(int timeoutMs)
{
    return QCoro::Task<void>();
}

QCoro::Task<ProviderResponse<BalanceResult>>
MempoolSpaceProvider::fetchBalanceFromAPI(const QString& address)
{
    return QCoro::Task<ProviderResponse<BalanceResult>>();
}

ProviderResponse<BalanceResult> MempoolSpaceProvider::parseBalanceResponse(
    const json& response)
{
    return ProviderResponse<BalanceResult>();
}

QCoro::Task<QString> MempoolSpaceProvider::waitForMessage(int timeoutMs)
{
    return QCoro::Task<QString>();
}

bool MempoolSpaceProvider::isValidBitcoinAddress(const QString& address)
{
    return false;
}
}
