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

#include "SolanaConnectionManager.h"

namespace Daitengu::Clients::Solana {

std::shared_ptr<SolanaConnectionManager> SolanaConnectionManager::instance_;
QMutex SolanaConnectionManager::instanceMutex_;

std::shared_ptr<SolanaConnectionManager> SolanaConnectionManager::instance()
{
    QMutexLocker locker(&instanceMutex_);
    if (!instance_) {
        instance_ = std::shared_ptr<SolanaConnectionManager>(
            new SolanaConnectionManager(), Deleter());
    }

    return instance_;
}

bool SolanaConnectionManager::connectToNode(const QUrl& url)
{
    if (connected_) {
        disconnectFromNode();
    }

    initialize();

    currentUrl_ = url;
    webSocket_.open(url);

    return true;
}

void SolanaConnectionManager::disconnectFromNode()
{
    cleanup();
    reconnectTimer_.stop();
    webSocket_.close();
}

bool SolanaConnectionManager::isConnected() const
{
    return connected_;
}

int SolanaConnectionManager::registerTransactionListener(
    const std::function<void(const json&)>& callback, const json& filterConfig)
{
    QMutexLocker locker(&listenerMutex_);

    int listenerId = generateListenerId();

    Subscription sub {
        .type = SubscriptionType::Transaction,
        .callback = callback,
        .filterConfig = filterConfig,
        .rpcId = -1,
        .wsId = -1,
    };

    subscriptions_[listenerId] = sub;

    if (connected_) {
        sendSubscriptionRequest(listenerId);
    }

    return listenerId;
}

int SolanaConnectionManager::registerAccountListener(
    std::string_view address, const std::function<void(const json&)>& callback)
{
    QMutexLocker locker(&listenerMutex_);

    int listenerId = generateListenerId();

    Subscription sub {
        .type = SubscriptionType::Account,
        .callback = callback,
        .address = QString::fromStdString(std::string(address)),
        .rpcId = -1,
        .wsId = -1,
    };

    subscriptions_[listenerId] = sub;

    if (connected_) {
        sendSubscriptionRequest(listenerId);
    }

    return listenerId;
}

int SolanaConnectionManager::registerProgramListener(std::string_view programId,
    const std::function<void(const json&)>& callback)
{
    QMutexLocker locker(&listenerMutex_);

    int listenerId = generateListenerId();

    Subscription sub {
        .type = SubscriptionType::Program,
        .callback = callback,
        .address = QString::fromStdString(std::string(programId)),
        .rpcId = -1,
        .wsId = -1,
    };

    subscriptions_[listenerId] = sub;

    if (connected_) {
        sendSubscriptionRequest(listenerId);
    }

    return listenerId;
}

void SolanaConnectionManager::unregisterListener(int listenerId)
{
    QMutexLocker locker(&listenerMutex_);

    if (!subscriptions_.contains(listenerId) || !connected_) {
        return;
    }

    Subscription& sub = subscriptions_[listenerId];

    if (sub.wsId <= 0) {
        return;
    }

    std::string_view method;

    switch (sub.type) {
    case SubscriptionType::Account:
        method = "accountUnsubscribe";
        break;
    case SubscriptionType::Transaction:
        method = "logsUnsubscribe";
        break;
    case SubscriptionType::Program:
        method = "programUnsubscribe";
        break;
    case SubscriptionType::Signature:
        method = "signatureUnsubscribe";
        break;
    default:
        qWarning() << "Unknown subscription type for unsubscribe";
        return;
    }

    json params = json::array();
    params.push_back(sub.wsId);

    sendJsonRpcRequest(method, params);

    subscriptions_.remove(listenerId);
}

void SolanaConnectionManager::destroy()
{
    QMutexLocker locker(&instanceMutex_);
    instance_.reset();
}

void SolanaConnectionManager::onConnected()
{
    connected_ = true;
    qDebug() << "Connected to Solana node";
    Q_EMIT connectionStatusChanged(true);

    resubscribeAll();
}

void SolanaConnectionManager::onDisconnected()
{
    connected_ = false;
    qDebug() << "Disconnected from Solana node";
    Q_EMIT connectionStatusChanged(false);

    QMutexLocker locker(&listenerMutex_);
    for (auto it = subscriptions_.begin(); it != subscriptions_.end(); ++it) {
        it.value().wsId = -1;
    }

    if (!currentUrl_.isEmpty() && !reconnectTimer_.isActive()) {
        reconnectTimer_.start();
    }
}

void SolanaConnectionManager::onTextMessageReceived(const QString& message)
{
    try {
        json json = json::parse(message.toStdString());

        qInfo() << message;

        if (json.contains("method") && json["method"] == "subscription") {
            processSubscriptionMessage(json);
            return;
        }

        if (json.contains("id") && json.contains("result")) {
            int id = json["id"].get<int>();

            QMutexLocker locker(&listenerMutex_);
            for (auto it = subscriptions_.begin(); it != subscriptions_.end();
                ++it) {
                if (it.value().rpcId == id) {
                    it.value().wsId = json["result"].get<int>();
                    it.value().rpcId = -1;
                    break;
                }
            }
        }

        if (json.contains("error")) {
            std::string errorMsg = json["error"]["message"].get<std::string>();
            Q_EMIT error(QString::fromStdString(errorMsg));
        }
    } catch (const std::exception& e) {
        qWarning() << "Failed to parse WebSocket message: " << e.what();
    }
}

void SolanaConnectionManager::onError(QAbstractSocket::SocketError error)
{
    qWarning() << "WebSocket error: " << webSocket_.errorString();
    Q_EMIT this->error("WebSocket error: " + webSocket_.errorString());
}

void SolanaConnectionManager::reconnect()
{
    if (!connected_ && !currentUrl_.isEmpty()) {
        qDebug() << "Attempting to reconnect to Solana node...";
        webSocket_.open(currentUrl_);
    }
}

SolanaConnectionManager::SolanaConnectionManager(QObject* parent)
    : QObject(parent)
{
    connect(&webSocket_, &QWebSocket::connected, this,
        &SolanaConnectionManager::onConnected);

    connect(&webSocket_, &QWebSocket::disconnected, this,
        &SolanaConnectionManager::onDisconnected);

    connect(&webSocket_, &QWebSocket::textMessageReceived, this,
        &SolanaConnectionManager::onTextMessageReceived);

    connect(&webSocket_,
        QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this,
        &SolanaConnectionManager::onError);

    reconnectTimer_.setInterval(5000);
    reconnectTimer_.setSingleShot(true);
    connect(&reconnectTimer_, &QTimer::timeout, this,
        &SolanaConnectionManager::reconnect);

    initialize();
}

SolanaConnectionManager::~SolanaConnectionManager()
{
    cleanup();

    if (connected_) {
        disconnectFromNode();
    }
}

void SolanaConnectionManager::initialize()
{
    connected_ = false;
    nextListenerId_ = 1;
    nextRpcId_ = 1;

    reconnectTimer_.stop();
    reconnectTimer_.setInterval(5000);

    qDebug() << "ConnectionManager initialized";
}

void SolanaConnectionManager::cleanup()
{
    QMutexLocker locker(&listenerMutex_);

    if (connected_) {
        for (auto it = subscriptions_.begin(); it != subscriptions_.end();
            ++it) {
            if (it.value().wsId > 0) {
                unregisterListener(it.key());
            }
        }
    }

    subscriptions_.clear();
    reconnectTimer_.stop();

    qDebug() << "ConnectionManager cleaned up";
}

int SolanaConnectionManager::sendJsonRpcRequest(
    std::string_view method, const json& params)
{
    int requestId = nextRpcId_++;

    json request;
    request["jsonrpc"] = "2.0";
    request["id"] = requestId;
    request["method"] = std::string(method);
    request["params"] = params;

    webSocket_.sendTextMessage(QString::fromStdString(request.dump()));

    return requestId;
}

void SolanaConnectionManager::processSubscriptionMessage(
    const json& notification)
{
    try {
        int subscriptionId = notification["params"]["subscription"].get<int>();
        json result = notification["params"]["result"];

        QMutexLocker locker(&listenerMutex_);
        for (auto it = subscriptions_.begin(); it != subscriptions_.end();
            ++it) {
            if (it.value().wsId == subscriptionId) {
                it.value().callback(result);
                break;
            }
        }
    } catch (const std::exception& e) {
        qWarning() << "Failed to process subscription message: " << e.what();
    }
}

void SolanaConnectionManager::resubscribeAll()
{
    QMutexLocker locker(&listenerMutex_);

    for (auto it = subscriptions_.begin(); it != subscriptions_.end(); ++it) {
        sendSubscriptionRequest(it.key());
    }
}

int SolanaConnectionManager::generateListenerId()
{
    return nextListenerId_++;
}

void SolanaConnectionManager::sendSubscriptionRequest(int listenerId)
{
    if (!subscriptions_.contains(listenerId) || !connected_) {
        return;
    }

    Subscription& sub = subscriptions_[listenerId];

    json params = json::array();
    std::string_view method;

    switch (sub.type) {
    case SubscriptionType::Transaction:
        method = "logsSubscribe";
        {
            json filter = "all";

            if (sub.filterConfig.is_object()
                && sub.filterConfig.contains("mentions")) {
                filter = { { "mentions", sub.filterConfig["mentions"] } };
            }

            params.push_back(filter);

            json config;
            config["commitment"] = "confirmed";
            params.push_back(config);
        }
        break;

    case SubscriptionType::Account:
        method = "accountSubscribe";
        params.push_back(sub.address.toStdString());
        {
            json config;
            config["encoding"] = "jsonParsed";
            config["commitment"] = "confirmed";
            params.push_back(config);
        }
        break;

    case SubscriptionType::Program:
        method = "programSubscribe";
        params.push_back(sub.address.toStdString());
        {
            json config;
            config["encoding"] = "jsonParsed";
            config["commitment"] = "confirmed";
            params.push_back(config);
        }
        break;

    default:
        break;
    }

    sub.rpcId = sendJsonRpcRequest(method, params);
}
}
