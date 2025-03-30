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
    if (!isValidBitcoinAddress(address)) {
        ProviderResponse<BalanceResult> response;
        response.success = false;
        response.error
            = ProviderError { 400, "Invalid Bitcoin address format" };
        co_return response;
    }

    if (!isConnected() && !isConnecting()) {
        bool connected = co_await initialize();
        if (!connected) {
            ProviderResponse<BalanceResult> response;
            response.success = false;
            response.error
                = ProviderError { 500, "Failed to connect to provider" };
            co_return response;
        }
    }

    co_return co_await fetchBalanceFromAPI(address);
}

QCoro::Task<ProviderResponse<TokenList>> MempoolSpaceProvider::getTokens(
    const QString& address)
{
    if (!isValidBitcoinAddress(address)) {
        ProviderResponse<TokenList> response;
        response.success = false;
        response.error
            = ProviderError { 400, "Invalid Bitcoin address format" };
        co_return response;
    }

    // TODO: ARC20
    ProviderResponse<TokenList> response;
    response.success = true;
    response.data = TokenList();

    co_return response;
}

QCoro::Task<ProviderResponse<bool>> MempoolSpaceProvider::isValidAddress(
    const QString& address)
{
    ProviderResponse<bool> response;

    try {
        response.success = true;
        response.data = isValidBitcoinAddress(address);
    } catch (const std::exception& e) {
        response.success = false;
        response.error = ProviderError { 500,
            QString("Error validating address: %1").arg(e.what()) };
        qCWarning(bcMonitor) << "Error validating address: " << e.what();
    }

    co_return response;
}

QCoro::Task<bool> MempoolSpaceProvider::subscribeToAddressChanges(
    const QString& address)
{
    if (!connected_) {
        qCWarning(bcMonitor)
            << "Cannot subscribe to address changes: not connected";
        co_return false;
    }

    if (!isValidBitcoinAddress(address)) {
        qCWarning(bcMonitor)
            << "Cannot subscribe to invalid address:" << address;
        co_return false;
    }

    json message = {
        { "action", "subscribe" },
        { "data", { QString("address:%1").arg(address).toStdString() } },
    };

    webSocket_->sendTextMessage(QString::fromStdString(message.dump()));

    subscribedAddresses_[address] = true;
    qCDebug(bcMonitor) << "Subscribed to address changes for" << address;

    co_return true;
}

QCoro::Task<void> MempoolSpaceProvider::unsubscribeFromAddressChanges(
    const QString& address)
{
    if (!connected_ || !subscribedAddresses_.contains(address)) {
        co_return;
    }

    json message = {
        { "action", "unsubscribe" },
        { "data", { QString("address:%1").arg(address).toStdString() } },
    };

    webSocket_->sendTextMessage(QString::fromStdString(message.dump()));

    subscribedAddresses_.remove(address);
    qCDebug(bcMonitor) << "Unsubscribed from address changes for" << address;

    co_return;
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

    webSocket_->open(QUrl(WS_BASE_URL));
    co_await qCoro(webSocket_.get(), &QWebSocket::connected);

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
    shutdownRequested_ = true;

    if (pingTimer_->isActive()) {
        pingTimer_->stop();
    }

    if (connectTimeoutTimer_->isActive()) {
        connectTimeoutTimer_->stop();
    }

    if (webSocket_->isValid()) {
        for (const auto& address : subscribedAddresses_.keys()) {
            try {
                co_await unsubscribeFromAddressChanges(address);
            } catch (...) {
                qCritical() << "Error in cleanup";
            }
        }

        if (webSocket_->state() != QAbstractSocket::UnconnectedState) {
            webSocket_->close();
            co_await qCoro(webSocket_.get(), &QWebSocket::disconnected);
        }
    }

    connecting_ = false;
    connected_ = false;

    Q_EMIT connectionStatusChanged(false);

    qCDebug(bcMonitor) << "Shutdown completed for " << providerName();
}

void MempoolSpaceProvider::onWebSocketConnected()
{
    connectTimeoutTimer_->stop();
    connecting_ = false;
    connected_ = true;

    qCInfo(bcMonitor) << "Connected to " << providerName() << " WebSocket";
    Q_EMIT connectionStatusChanged(true);

    setupPingPong();
}

void MempoolSpaceProvider::onWebSocketDisconnected()
{
    connecting_ = false;
    connected_ = false;

    qCInfo(bcMonitor) << "Disconnected from " << providerName() << " WebSocket";
    Q_EMIT connectionStatusChanged(false);

    if (!shutdownRequested_) {
        // Another way
        /*QCoro::sleepFor(std::chrono::seconds(5)).then([this]() {
            if (!shutdownRequested_ && !connected_ && !connecting_) {
                initialize();
            }
        });*/

        QTimer::singleShot(5000, this, [this]() {
            if (!shutdownRequested_ && !connected_ && !connecting_) {
                initialize();
            }
        });
    }
}

void MempoolSpaceProvider::onWebSocketTextMessageReceived(
    const QString& message)
{
    try {
        auto data = json::parse(message.toStdString());

        if (data.contains("address")) {
            QString address
                = QString::fromStdString(data["address"].get<std::string>());

            qCDebug(bcMonitor) << "Received address update for " << address;

            QMetaObject::invokeMethod(this, [this, address]() {
                getBalance(address).then(
                    [this, address](ProviderResponse<BalanceResult> response) {
                        if (response.success && response.data) {
                            Q_EMIT balanceChanged(
                                address, response.data.value());
                        }
                    });

                getTokens(address).then(
                    [this, address](ProviderResponse<TokenList> response) {
                        if (response.success && response.data) {
                            Q_EMIT tokensChanged(
                                address, response.data.value());
                        }
                    });
            });
        }
    } catch (const std::exception& e) {
        qCWarning(bcMonitor) << "Error parsing WebSocket message: " << e.what();
    }
}

void MempoolSpaceProvider::onWebSocketError(
    QAbstractSocket::SocketError socketError)
{
    QString errorMsg
        = QString("WebSocket error: %1").arg(webSocket_->errorString());
    qCWarning(bcMonitor) << errorMsg;

    Q_EMIT error(errorMsg);

    if (connecting_) {
        connecting_ = false;
        connectTimeoutTimer_->stop();
    }
}

void MempoolSpaceProvider::onSslErrors(const QList<QSslError>& errors)
{
    QString errorStr = "SSL Errors:";
    for (const auto& error : errors) {
        errorStr += " " + error.errorString();
    }
    qCWarning(bcMonitor) << errorStr;

    Q_EMIT error(errorStr);

    // TODO: Read from settings
    bool shouldIgnore = false;

    if (shouldIgnore) {
        webSocket_->ignoreSslErrors();
    } else {
        webSocket_->abort();
        connecting_ = false;
        connected_ = false;
        connectTimeoutTimer_->stop();
        Q_EMIT connectionStatusChanged(false);
    }
}

void MempoolSpaceProvider::setupPingPong()
{
    connect(pingTimer_.get(), &QTimer::timeout, this, [this]() {
        checkPingPongHealth();

        if (webSocket_->isValid() && connected_) {
            webSocket_->ping();
            qCDebug(bcMonitor) << "Send ping to " << providerName();
        }
    });

    connect(webSocket_.get(), &QWebSocket::pong, this,
        [this](quint64 elapsedTime, const QByteArray&) {
            lastPongTime_ = QDateTime::currentDateTime();
            consecutivePingFailures_ = 0;
            qCDebug(bcMonitor) << "Received pong from " << providerName()
                               << " after " << elapsedTime << " ms";
        });

    pingTimer_->start(PING_INTERVAL_MS);
    lastPongTime_ = QDateTime::currentDateTime();
}

void MempoolSpaceProvider::checkPingPongHealth()
{
    if (!lastPongTime_.isValid()) {
        lastPongTime_ = QDateTime::currentDateTime();
        return;
    }

    auto elapsed = lastPongTime_.msecsTo(QDateTime::currentDateTime());
    if (elapsed > PING_TIMEOUT_MS) {
        consecutivePingFailures_++;
        qCWarning(bcMonitor)
            << "WebSocket ping timeout detected for " << providerName()
            << " - Failure count: " << consecutivePingFailures_;

        if (consecutivePingFailures_ >= 3) {
            qCWarning(bcMonitor)
                << "Multiple ping failures, reconnecting to " << providerName();

            QMetaObject::invokeMethod(
                this,
                [this]() {
                    webSocket_->abort();
                    connected_ = false;
                    Q_EMIT connectionStatusChanged(false);

                    QTimer::singleShot(1000, this, [this]() {
                        if (!shutdownRequested_) {
                            initialize();
                        }
                    });
                },
                Qt::QueuedConnection);
        }
    }
}

QCoro::Task<void> MempoolSpaceProvider::waitForConnection(int timeoutMs)
{
    if (connected_) {
        co_return;
    }

    if (!connecting_) {
        co_return;
    }

    try {
        co_await qCoro(webSocket_.get(), &QWebSocket::connected,
            std::chrono::milliseconds(timeoutMs));
    } catch (const std::exception& e) {
        qCWarning(bcMonitor)
            << "Waiting for WebSocket connection - Error: " << e.what();
    }

    co_return;
}

QCoro::Task<ProviderResponse<BalanceResult>>
MempoolSpaceProvider::fetchBalanceFromAPI(const QString& address)
{
    ProviderResponse<BalanceResult> response;

    QString url = QString("%1/address/%2").arg(API_BASE_URL, address);
    QNetworkRequest request { QUrl(url) };
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    try {
        QNetworkReply* reply = networkManager_->get(request);
        co_await qCoro(reply, &QNetworkReply::finished);

        if (reply->error() == QNetworkReply::NoError) {
            try {
                auto data = json::parse(reply->readAll().toStdString());
                response = parseBalanceResponse(data);
            } catch (const std::exception& e) {
                response.success = false;
                response.error = ProviderError { 500,
                    QString("JSON parsing error: %1").arg(e.what()) };
                qCWarning(bcMonitor) << "JSON parsing error: " << e.what();
            }
        } else {
            response.success = false;
            response.error = ProviderError { static_cast<int>(reply->error()),
                QString("Network error: %1").arg(reply->errorString()) };
            qCWarning(bcMonitor) << "Network error: " << reply->errorString();
        }

        reply->deleteLater();
    } catch (const std::exception& e) {
        response.success = false;
        response.error = ProviderError { 500,
            QString("Unexpected error: %1").arg(e.what()) };
        qCWarning(bcMonitor) << "Unexpected error: " << e.what();
    }

    co_return response;
}

ProviderResponse<BalanceResult> MempoolSpaceProvider::parseBalanceResponse(
    const json& response)
{
    ProviderResponse<BalanceResult> result;

    try {
        if (response.contains("chain_stats")
            && response["chain_stats"].contains("funded_txo_sum")
            && response.contains("mempool_stats")
            && response["mempool_stats"].contains("funded_txo_sum")) {

            uint64_t confirmed
                = response["chain_stats"]["funded_txo_sum"].get<uint64_t>()
                - response["chain_stats"]["spent_txo_sum"].get<uint64_t>();

            uint64_t unconfirmed
                = response["mempool_stats"]["funded_txo_sum"].get<uint64_t>()
                - response["mempool_stats"]["spent_txo_sum"].get<uint64_t>();

            uint64_t totalSats = confirmed + unconfirmed;

            double btcAmount = static_cast<double>(totalSats) / 100000000.0;

            result.success = true;
            result.data = btcAmount;

            qCDebug(bcMonitor) << "Parsed balance: " << btcAmount << " BTC";
        } else {
            result.success = false;
            result.error = ProviderError { 400, "Invalid response format" };
            qCWarning(bcMonitor) << "Invalid balance response format";
        }
    } catch (const std::exception& e) {
        result.success = false;
        result.error = ProviderError { 500,
            QString("Error parsing balance: %1").arg(e.what()) };
        qCWarning(bcMonitor) << "Error parsing balance: " << e.what();
    }

    return result;
}

QCoro::Task<QString> MempoolSpaceProvider::waitForMessage(int timeoutMs)
{
    try {
        auto opt
            = co_await qCoro(webSocket_.get(), &QWebSocket::textMessageReceived,
                std::chrono::milliseconds(timeoutMs));
        if (opt.has_value()) {
            co_return *opt;
        } else {
            qCWarning(bcMonitor)
                << "No WebSocket message received (empty optional)";
        }
        // co_return co_await qCoro(webSocket_.get(),
        // &QWebSocket::textMessageReceived);
    } catch (const std::exception& e) {
        qCWarning(bcMonitor)
            << "Waiting for WebSocket message error: " << e.what();
    }

    co_return QString();
}

bool MempoolSpaceProvider::isValidBitcoinAddress(const QString& address)
{
    std::string stdAddress = address.toStdString();
    std::string_view addressView = stdAddress;

    return BitcoinWallet::isValid(addressView);
}
}
