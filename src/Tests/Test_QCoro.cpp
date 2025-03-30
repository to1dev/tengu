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
#include <random>

#include <QCoreApplication>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>
#include <QWebSocket>
#include <QtConcurrent>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

#include "qcoro/QCoroCore"
#include "qcoro/QCoroNetworkReply"
#include "qcoro/QCoroTask"
#include "qcoro/QCoroWebSocket"

#include "semver.hpp"

QCoro::Task<> startTask()
{
    const auto data = co_await QtConcurrent::run([]() {
        QVector<std::uint64_t> data;
        std::random_device rd {};
        std::mt19937 gen { rd() };
        data.reserve(10'000'000);
        for (int i = 0; i < 10'000'000; ++i) {
            data.push_back(gen());
        }

        return data;
    });

    std::cout << "Generated " << data.size() << " random numbers" << std::endl;

    const auto sum = co_await QtConcurrent::filteredReduced<std::uint64_t>(
        data, [](const auto&) { return true; },
        [](std::uint64_t& interm, std::uint64_t val) { interm += val; },
        QtConcurrent::UnorderedReduce);

    std::cout << "Calculated result: " << sum << std::endl;

    co_return;
}

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

QCoro::Task<void> testReply()
{
    QNetworkRequest request { QUrl(
        "https://api.github.com/repos/vercel/next.js/releases/latest") };

    QNetworkAccessManager manager;
    auto reply = std::unique_ptr<QNetworkReply>(manager.get(request));

    co_await qCoro(reply.get()).waitForFinished();

    if (reply->isFinished() && reply->error() == QNetworkReply::NoError) {
        json data = json::parse(reply->readAll().constData());

        std::string tagName = data["tag_name"].get<std::string>();
        if (!tagName.empty() && tagName[0] == 'v') {
            tagName.erase(0, 1);
        }

        auto ver = semver::version { tagName };

        std::cout << ver << std::endl;
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

        /*auto task = testWebSocketCoro();

        auto quit = [&app, task = std::move(task)]() -> QCoro::Task<void> {
            try {
                co_await task;
                qDebug() << "Completed";
            } catch (const std::exception& e) {
                qDebug() << "Error: " << e.what();
            }
            app.quit();
        };

        quit();*/

        // auto task = startTask();
        auto task = testReply();
        task.then([&app]() {
            qDebug() << "Completed, quiting...";
            app.quit();
        });
    });

    return app.exec();
}
