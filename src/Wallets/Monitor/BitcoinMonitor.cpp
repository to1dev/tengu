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

#include "BitcoinMonitor.h"

namespace Daitengu::Wallets {

BitcoinMonitor::BitcoinMonitor(
    const QString& rpcUrl, const QString& wsUrl, QObject* parent)
    : BlockchainMonitor(parent)
    , rpcUrl_(rpcUrl)
    , wsUrl_(wsUrl)
    , webSocket_(std::make_unique<QWebSocket>())
    , networkManager_(std::make_unique<QNetworkAccessManager>(this))
    , reconnectTimer_(std::make_unique<QTimer>(this))
{
    webSocket_->setParent(this);
    QObject::connect(webSocket_.get(), &QWebSocket::connected, this,
        &BitcoinMonitor::onWebSocketConnected);
    QObject::connect(webSocket_.get(), &QWebSocket::disconnected, this,
        &BitcoinMonitor::onWebSocketDisconnected);
    QObject::connect(webSocket_.get(), &QWebSocket::textMessageReceived, this,
        &BitcoinMonitor::onWebSocketTextMessageReceived);
    QObject::connect(webSocket_.get(),
        qOverload<QAbstractSocket::SocketError>(&QWebSocket::error), this,
        &BitcoinMonitor::onWebSocketError);
    QObject::connect(webSocket_.get(), &QWebSocket::sslErrors, this,
        &BitcoinMonitor::onSslErrors);

    reconnectTimer_->setInterval(5000);
    QObject::connect(reconnectTimer_.get(), &QTimer::timeout, this, [this]() {
        if (!isConnected()) {
            connect();
        }
    });
}

BitcoinMonitor::~BitcoinMonitor()
{
    disconnect();
}

bool BitcoinMonitor::connect()
{
    return false;
}

void BitcoinMonitor::disconnect()
{
}

void BitcoinMonitor::refreshBalance()
{
}

void BitcoinMonitor::refreshTokens()
{
}

bool BitcoinMonitor::isValidAddress(const QString& address) const
{
    return false;
}

void BitcoinMonitor::onWebSocketConnected()
{
}

void BitcoinMonitor::onWebSocketDisconnected()
{
}

void BitcoinMonitor::onWebSocketTextMessageReceived(const QString& message)
{
}

void BitcoinMonitor::onNetworkReplyFinished()
{
}

void BitcoinMonitor::onWebSocketError(QAbstractSocket::SocketError error)
{
}

void BitcoinMonitor::onSslErrors(const QList<QSslError>& errors)
{
}

void BitcoinMonitor::onNetworkError(QNetworkReply::NetworkError error)
{
}

void BitcoinMonitor::setupConnection()
{
}

void BitcoinMonitor::subscribeToAddress()
{
}

void BitcoinMonitor::checkPingPongHealth()
{
    if (!lastPongTime_.isValid())
        return;

    auto elapsed = lastPongTime_.msecsTo(QDateTime::currentDateTime());
    if (elapsed > 90000) {
        consecutivePingFailures_++;
        qWarning() << "WebSocket ping timeout detected";

        if (consecutivePingFailures_ >= 3) {
            qWarning() << "Multiple ping failures, reconnecting...";
            disconnect();
            QTimer::singleShot(1000, this, [this]() { connect(); });
        }
    }
}

void BitcoinMonitor::setupPingPong()
{
    pingTimer_ = std::make_unique<QTimer>(this);
    QObject::connect(pingTimer_.get(), &QTimer::timeout, this, [this]() {
        checkPingPongHealth();

        if (webSocket_->isValid()
            && webSocket_->state() == QAbstractSocket::ConnectedState) {
            lastPingTime_ = QDateTime::currentDateTime();
            webSocket_->ping();
        }
    });

    QObject::connect(webSocket_.get(), &QWebSocket::pong, this,
        [this](quint64 elapsedTime, const QByteArray&) {
            lastPongTime_ = QDateTime::currentDateTime();
            pingLatency_ = elapsedTime;
            consecutivePingFailures_ = 0;
        });

    pingTimer_->start(30000);
}

void BitcoinMonitor::sendJsonRpcRequest(
    const QString& method, const json& params)
{
}

void BitcoinMonitor::processBalanceResponse(const json& response)
{
}

void BitcoinMonitor::processTokenResponse(const json& response)
{
}

bool BitcoinMonitor::isValidBitcoinAddress(const QString& address)
{
    return false;
}
}
