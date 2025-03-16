#include "Client.h"

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

#include "Client.h"

namespace Daitengu::Clients::Solana {

Client::Client(QObject* parent)
    : QObject(parent)
{
}

Client::~Client()
{
}

bool Client::isConnected()
{
    return (QAbstractSocket::ConnectedState == webSocket_->state());
}

void Client::open()
{
    if (!isConnected()) {
        qDebug() << "Trying to connect to: " << wsUrl_;
        QUrl url(wsUrl_);
        if (url.isValid()) {
            webSocket_->open(url);
        } else {
            qDebug() << "Invalid endpoint: " << wsUrl_;
        }
    } else {
        qDebug() << "Already connected";
    }
}

void Client::close()
{
    waitingForResponse_ = false;
    working_ = false;
    webSocket_->close();
}

bool Client::isWorking() const
{
    return working_ == true;
}

void Client::onConnected()
{
    qDebug() << ">> WebSocket Connected...";

    reconnectAttempts_ = 0;
    Q_EMIT connected();

    sendNextCommand();
}

void Client::onDisconnected()
{
    qDebug() << ">> WebSocket Disconnected...";
    working_ = false;
}

void Client::sendNextCommand()
{
    if (!commandQueue_.isEmpty() && !waitingForResponse_) {
        if (isConnected()) {
            Command cmd = commandQueue_.head();
            webSocket_->sendTextMessage(cmd.command);
            waitingForResponse_ = true;
        } else {
            open();
        }
    } else if (commandQueue_.isEmpty()) {
        working_ = false;
    }
}

}
