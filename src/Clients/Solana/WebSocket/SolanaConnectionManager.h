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
#include <functional>

#include <QDebug>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSet>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QWebSocket>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace Daitengu::Clients::Solana {

class SolanaConnectionManager : public QObject {
    Q_OBJECT

public:
    static SolanaConnectionManager* instance();

    bool connectToNode(const QUrl& url);

    void disconnectFromNode();

    bool isConnected() const;

    enum class SubscriptionType {
        Transaction,
        Account,
        Program,
        Signature,
    };

    int registerTransactionListener(
        const std::function<void(const json&)>& callback,
        const json& filterConfig = nullptr);

    int registerAccountListener(const QString& address,
        const std::function<void(const json&)>& callback);

    int registerProgramListener(const QString& programId,
        const std::function<void(const json&)>& callback);

    void unregisterListener(int listenerId);

    void destroy();

Q_SIGNALS:
    void doConnect(const QUrl& url);
    void doDisconnect();
    void doSendMessage(const QString& message);

    void connectionStatusChanged(bool connected);
    void error(const QString& errorMessage);

private Q_SLOTS:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onError(QAbstractSocket::SocketError error);
    void sendMessage(const QString& message);
    void reconnect();

private:
    explicit SolanaConnectionManager();
    ~SolanaConnectionManager();

    SolanaConnectionManager(const SolanaConnectionManager&) = delete;
    SolanaConnectionManager& operator=(const SolanaConnectionManager&) = delete;

    void initialize();
    void cleanup();

    struct Subscription {
        SubscriptionType type;
        std::function<void(const nlohmann::json&)> callback;
        nlohmann::json filterConfig;
        QString address;
        int rpcId;
        int wsId;
    };

    int sendJsonRpcRequest(const QString& method, const json& params);

    void processSubscriptionMessage(const json& notification);

    void resubscribeAll();

    int generateListenerId();

    void sendSubscriptionRequest(int listenerId);

private:
    static SolanaConnectionManager* instance_;
    static QMutex instanceMutex_;

    QWebSocket webSocket_;
    QThread workerThread_;
    QTimer reconnectTimer_;

    QUrl currentUrl_;

    std::atomic<bool> connected_ { false };

    QMutex listenerMutex_;
    QMap<int, Subscription> subscriptions_;

    std::atomic<int> nextListenerId_ { 1 };
    std::atomic<int> nextRpcId_ { 1 };
};

}
