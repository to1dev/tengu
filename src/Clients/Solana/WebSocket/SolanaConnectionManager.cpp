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

#include <iostream>

#include <QMutexLocker>

#include "SolanaConnectionManager.h"

namespace Daitengu::Clients::Solana {

std::shared_ptr<SolanaConnectionManager> SolanaConnectionManager::instance_;
QMutex SolanaConnectionManager::instanceMutex_;

std::shared_ptr<SolanaConnectionManager> SolanaConnectionManager::instance()
{
    QMutexLocker locker(&instanceMutex_);
    if (!instance_) {
        instance_.reset(new SolanaConnectionManager(), Deleter());
    }
    return instance_;
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
    qDebug() << "[SolanaConnectionManager] initialized";
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

    qDebug() << "[SolanaConnectionManager] cleaned up";
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

void SolanaConnectionManager::destroy()
{
    QMutexLocker locker(&instanceMutex_);
    instance_.reset();
}

int SolanaConnectionManager::registerLogsListener(
    const std::function<void(const json&)>& callback, const json& filterConfig)
{
    QMutexLocker locker(&listenerMutex_);

    int listenerId = generateListenerId();

    Subscription sub {
        .type = SubscriptionType::Logs,
        .callback = callback,
        .filterConfig = filterConfig,
        .address = QString(),
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
        .filterConfig = nullptr,
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
        .filterConfig = nullptr,
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
        subscriptions_.remove(listenerId);
        return;
    }

    Subscription& sub = subscriptions_[listenerId];

    if (sub.wsId <= 0) {
        subscriptions_.remove(listenerId);
        return;
    }

    std::string_view method;
    switch (sub.type) {
    case SubscriptionType::Logs:
        method = "logsUnsubscribe";
        break;
    case SubscriptionType::Account:
        method = "accountUnsubscribe";
        break;
    case SubscriptionType::Program:
        method = "programUnsubscribe";
        break;
    case SubscriptionType::Signature:
        method = "signatureUnsubscribe";
        break;
    }

    json params = json::array();
    params.push_back(sub.wsId);
    sendJsonRpcRequest(method, params);

    subscriptions_.remove(listenerId);
}

void SolanaConnectionManager::requestTransactionDetails(
    const std::string& signature, std::function<void(const json&)> callback)
{
    json params = json::array();
    params.push_back(signature);

    std::cout << "fuck off: " << signature << std::endl;

    // "jsonParsed" / "json" / "base64"
    json config;
    config["encoding"] = "jsonParsed";
    config["commitment"] = "confirmed";
    params.push_back(config);

    int reqId = sendJsonRpcRequest("getTransaction", params);
    {
        QMutexLocker locker(&requestMutex_);
        PendingRequest pr;
        pr.callback = callback;
        pendingRequests_[reqId] = pr;
    }
}

void SolanaConnectionManager::onConnected()
{
    connected_ = true;
    qDebug() << "[SolanaConnectionManager] WebSocket connected";
    Q_EMIT connectionStatusChanged(true);
    resubscribeAll();
}

void SolanaConnectionManager::onDisconnected()
{
    connected_ = false;
    qDebug() << "[SolanaConnectionManager] WebSocket disconnected";
    Q_EMIT connectionStatusChanged(false);

    {
        QMutexLocker locker(&listenerMutex_);
        for (auto it = subscriptions_.begin(); it != subscriptions_.end();
            ++it) {
            it.value().wsId = -1;
        }
    }

    if (!currentUrl_.isEmpty() && !reconnectTimer_.isActive()) {
        reconnectTimer_.start();
    }
}

void SolanaConnectionManager::onTextMessageReceived(const QString& message)
{
    json msg;
    try {
        msg = json::parse(message.toStdString());
        // std::cout << msg.dump(4) << std::endl;
    } catch (...) {
        qWarning() << "[SolanaConnectionManager] parse fail" << message;
        return;
    }

    qDebug() << "fuck off";
    if (msg.contains("method") && msg["method"] == "logsNotification") {

        processSubscriptionMessage(msg);
        return;
    }

    if (msg.contains("id")) {
        int id = msg["id"].get<int>();

        {
            QMutexLocker locker(&listenerMutex_);
            for (auto it = subscriptions_.begin(); it != subscriptions_.end();
                ++it) {
                if (it.value().rpcId == id) {
                    if (msg.contains("result")) {
                        it.value().wsId = msg["result"].get<int>();
                    }
                    it.value().rpcId = -1;
                    return;
                }
            }
        }

        {
            QMutexLocker locker(&requestMutex_);
            if (pendingRequests_.contains(id)) {
                auto pr = pendingRequests_.take(id);

                if (msg.contains("error")) {
                    std::string e = msg["error"]["message"].get<std::string>();
                    qWarning()
                        << "[SolanaConnectionManager] getTransaction error:"
                        << QString::fromStdString(e);
                    pr.callback(json());
                } else if (msg.contains("result")) {
                    pr.callback(msg["result"]);
                }
                return;
            }
        }
    }

    if (msg.contains("error")) {
        std::string e = msg["error"]["message"].get<std::string>();
        Q_EMIT error(QString::fromStdString(e));
    }
}

void SolanaConnectionManager::onError(QAbstractSocket::SocketError error)
{
    qWarning() << "[SolanaConnectionManager] WebSocket error:"
               << webSocket_.errorString();
    Q_EMIT this->error("WebSocket error: " + webSocket_.errorString());
}

void SolanaConnectionManager::reconnect()
{
    if (!connected_ && !currentUrl_.isEmpty()) {
        qDebug() << "[SolanaConnectionManager] Reconnect attempt...";
        webSocket_.open(currentUrl_);
    }
}

int SolanaConnectionManager::sendJsonRpcRequest(
    std::string_view method, const json& params)
{
    int rid = nextRpcId_++;
    json req;
    req["jsonrpc"] = "2.0";
    req["id"] = rid;
    req["method"] = std::string(method);
    req["params"] = params;

    std::string payload = req.dump();
    webSocket_.sendTextMessage(QString::fromStdString(payload));
    return rid;
}

void SolanaConnectionManager::processSubscriptionMessage(
    const json& notification)
{
    try {
        int subId = notification["params"]["subscription"].get<int>();
        json result = notification["params"]["result"];

        QMutexLocker locker(&listenerMutex_);
        for (auto it = subscriptions_.begin(); it != subscriptions_.end();
            ++it) {
            if (it.value().wsId == subId) {
                it.value().callback(result);
                break;
            }
        }
    } catch (...) {
        qWarning()
            << "[SolanaConnectionManager] processSubscriptionMessage exception";
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
    if (!subscriptions_.contains(listenerId) || !connected_)
        return;

    Subscription& sub = subscriptions_[listenerId];
    json params = json::array();
    std::string_view method;

    switch (sub.type) {
    case SubscriptionType::Logs:
        method = "logsSubscribe";
        if (!sub.filterConfig.is_null()) {
            // e.g. {"mentions":["addr1","addr2"]}
            params.push_back(sub.filterConfig);
        } else {
            params.push_back("all");
        }
        {
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
    case SubscriptionType::Signature:
        method = "signatureSubscribe";
        params.push_back(sub.address.toStdString());
        {
            json config;
            config["commitment"] = "confirmed";
            params.push_back(config);
        }
        break;
    }

    sub.rpcId = sendJsonRpcRequest(method, params);
}
}
