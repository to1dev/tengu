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

#include <exception>
#include <iostream>

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QUrl>
#include <QWebSocket>

#include "qcoro/QCoroCore"
#include "qcoro/QCoroSignal"
#include "qcoro/QCoroTask"
#include "qcoro/QCoroWebSocket"

QCoro::Task<void> testWebSocketCoro()
{
    QWebSocket socket;

    QUrl serverUrl("wss://echo.websocket.org");

    socket.open(serverUrl);

    qDebug() << "Connecting to" << serverUrl.toString() << "...";

    try {
        co_await qCoro(&socket, &QWebSocket::connected);
        qDebug() << "Already connected...";

        QString message1 = "Hello, WebSocket with QCoro!";
        qDebug() << "Sending message: " << message1;
        socket.sendTextMessage(message1);

        QString reply1
            = co_await qCoro(&socket, &QWebSocket::textMessageReceived);
        qDebug() << "Received: " << reply1;

        QString message2 = "Hello, this is the second message";
        qDebug() << "Sending message: " << message2;
        socket.sendTextMessage(message2);

        QString reply2
            = co_await qCoro(&socket, &QWebSocket::textMessageReceived);
        qDebug() << "Received: " << reply2;

        auto disconnectFuture = qCoro(&socket, &QWebSocket::disconnected);

        socket.close();

        co_await disconnectFuture;
        qDebug() << "Already disconnected...";
    } catch (const std::exception& e) {
        qCritical() << "An unexpected error occurred:" << e.what();
    }

    co_return;
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    QTimer::singleShot(0, &app, [&app]() {
        /*auto task = testWebSocketCoro();

        task.then([&app]() {
            qDebug() << "Completed, quiting...";
            app.quit();
        });*/

        auto task = testWebSocketCoro();

        auto quit = [&app, task = std::move(task)]() -> QCoro::Task<void> {
            try {
                co_await task;
                qDebug() << "Completed";
            } catch (const std::exception& e) {
                qDebug() << "Error: " << e.what();
            }
            app.quit();
        };

        quit();
    });

    return app.exec();
}
