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

#include <functional>
#include <iostream>
#include <optional>

#include <QDebug>
#include <QObject>
#include <QQueue>
#include <QString>
#include <QTimer>
#include <QWebSocket>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace Daitengu::Clients::Solana {

class Client : public QObject {
    Q_OBJECT

    struct Command {
        QString command;
        std::function<void(const json&)> callback;
    };

public:
    explicit Client(QObject* parent = nullptr);
    ~Client();

    bool isConnected();
    void open();
    void close();

    bool isWorking() const;

Q_SIGNALS:
    void connected();
    void start();
    void done();

private Q_SLOTS:
    void onConnected();
    void onDisconnected();

private:
    void sendNextCommand();

private:
    std::unique_ptr<QWebSocket> webSocket_ { std::make_unique<QWebSocket>() };
    QString rpcUrl_;
    QString wsUrl_;

    int reconnectAttempts_ { 0 };
    bool reconnected_ { false };
    bool waitingForResponse_ { false };
    bool working_ { false };
    QQueue<Command> commandQueue_;
};

}
