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

#include "qcoro5/qcoro/QCoroCore"
#include "qcoro5/qcoro/QCoroNetworkReply"
#include "qcoro5/qcoro/QCoroTask"
#include "qcoro5/qcoro/QCoroWebSocket"

#include "semver.hpp"
using semver::operator""_version;

#include "Utils/AutoUpdater.h"

using namespace Daitengu::Utils;

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

QCoro::Task<void> testUpdater()
{
    AutoUpdater updater;
    updater.setGitHubRepo("to1dev", "tengu");
    updater.setCurrentVersion("0.0.0");
    updater.setLogCallback(
        [](AutoUpdater::Error error, const std::string& msg) {
            std::cerr << "Error " << static_cast<int>(error) << ": " << msg
                      << std::endl;
        });

    try {
        semver::version v1 = "0.0.1-alpha.reforge"_version;
        semver::version v2 = "0.0.1-alpha.1"_version;
        semver::version v3 = "0.0.1+build.123"_version;

        std::cout << "v1: " << v1 << std::endl; // 0.0.1-alpha.reforge
        std::cout << "v2: " << v2 << std::endl; // 0.0.1-alpha.1
        std::cout << "v3: " << v3 << std::endl; // 0.0.1+build.123

        std::cout << std::boolalpha;
        std::cout << "v1 < v2: " << (v1 < v2) << std::endl;   // true
        std::cout << "v1 == v3: " << (v1 == v3) << std::endl; // false
        std::cout
            << "v1 == v3 (exclude prerelease): "
            << semver::comparators::equal_to(v1, v3,
                   semver::comparators::comparators_option::exclude_prerelease)
            << std::endl; // true

        std::cout << "v1 satisfies ^0.0.1: "
                  << semver::range::satisfies(v1, "^0.0.1",
                         semver::range::satisfies_option::include_prerelease)
                  << std::endl; // true
        std::cout << "v2 satisfies 0.0.1 - 0.0.2: "
                  << semver::range::satisfies(v2, "0.0.1 - 0.0.2",
                         semver::range::satisfies_option::include_prerelease)
                  << std::endl; // true

        std::cout << "semver_version: " << semver::semver_version
                  << std::endl; // 1.0.0

        AutoUpdater::Version version;
        version.version = "0.0.2-beta";
        auto semver = version.asSemver();
        if (semver) {
            std::cout << "Parsed version: " << *semver
                      << std::endl; // 0.0.2-beta
            std::cout << "Is newer: " << (*semver > v1) << std::endl; // true
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    auto checkTask = updater.checkForUpdates();
    auto v = co_await checkTask;
    if (v) {
        auto downloadTask = updater.downloadUpdate(
            *v, [](const AutoUpdater::UpdateProgress& progress) {
                std::cout << "Download: " << progress.percentage() << "%\n";
            });
        auto filePath = co_await downloadTask;
        if (filePath) {
            std::cout << "Dummy install " << *filePath << std::endl;
        }
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
        // auto task = testReply();
        auto task = testUpdater();
        task.then([&app]() {
            qDebug() << "Completed, quiting...";
            app.quit();
        });
    });

    return app.exec();
}
