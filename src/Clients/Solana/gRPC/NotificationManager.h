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

#include <memory>

#include <QNetworkAccessManager>
#include <QString>

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include <qcoro/QCoro>

namespace Daitengu::Clients::Solana::gRPC {

class NotificationPlugin {
public:
    virtual ~NotificationPlugin() = default;
    virtual QCoro::Task<void> sendNotification(const QString& message) = 0;
    virtual std::string name() const = 0;
};

class TelegramPlugin : public NotificationPlugin {
public:
    TelegramPlugin(const QString& botToken, const QString& chatId,
        QNetworkAccessManager* mgr);
    QCoro::Task<void> sendNotification(const QString& message) override;

    std::string name() const override
    {
        return "telegram";
    }

private:
    QString botToken_;
    QString chatId_;
    QNetworkAccessManager* networkManager_;
};

class DiscordPlugin : public NotificationPlugin {
public:
    DiscordPlugin(const QString& webhookUrl, QNetworkAccessManager* mgr);
    QCoro::Task<void> sendNotification(const QString& message) override;

    std::string name() const override
    {
        return "discord";
    }

private:
    QString webhookUrl_;
    QNetworkAccessManager* networkManager_;
};

class NotificationManager : public QObject {
    Q_OBJECT
public:
    explicit NotificationManager(QObject* parent = nullptr);
    ~NotificationManager();

    void addPlugin(std::unique_ptr<NotificationPlugin> plugin);
    void removePlugin(const std::string& name);
    QCoro::Task<void> sendBatchNotifications(const QString& message);

private:
    QNetworkAccessManager networkManager_;
    std::unordered_map<std::string, std::unique_ptr<NotificationPlugin>>
        plugins_;
};
}
