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

#include "NotificationManager.h"

namespace Daitengu::Clients::Solana::gRPC {

TelegramPlugin::TelegramPlugin(
    const QString& botToken, const QString& chatId, QNetworkAccessManager* mgr)
    : botToken_(botToken)
    , chatId_(chatId)
    , networkManager_(mgr)
{
}

QCoro::Task<void> TelegramPlugin::sendNotification(const QString& message)
{
    QString url
        = QString("https://api.telegram.org/bot%1/sendMessage").arg(botToken_);
    QNetworkRequest request { url };
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    nlohmann::json json;
    json["chat_id"] = chatId_.toStdString();
    json["text"] = message.toStdString();

    QNetworkReply* reply = networkManager_->post(
        request, QByteArray::fromStdString(json.dump()));
    co_await qCoro(reply, &QNetworkReply::finished);
    if (reply->error() != QNetworkReply::NoError) {
        spdlog::error("Telegram notification failed: {}",
            reply->errorString().toStdString());
    }
    reply->deleteLater();

    co_return;
}

DiscordPlugin::DiscordPlugin(
    const QString& webhookUrl, QNetworkAccessManager* mgr)
    : webhookUrl_(webhookUrl)
    , networkManager_(mgr)
{
}

QCoro::Task<void> DiscordPlugin::sendNotification(const QString& message)
{
    QNetworkRequest request { webhookUrl_ };
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    nlohmann::json json;
    json["content"] = message.toStdString();

    QNetworkReply* reply = networkManager_->post(
        request, QByteArray::fromStdString(json.dump()));

    co_await qCoro(reply, &QNetworkReply::finished);
    if (reply->error() != QNetworkReply::NoError) {
        spdlog::error("Discord notification failed: {}",
            reply->errorString().toStdString());
    }
    reply->deleteLater();

    co_return;
}

NotificationManager::NotificationManager(QObject* parent)
    : QObject(parent)
{
}

NotificationManager::~NotificationManager() = default;

void NotificationManager::addPlugin(std::unique_ptr<NotificationPlugin> plugin)
{
    plugins_[plugin->name()] = std::move(plugin);
    spdlog::info("Notification plugin added: {}", plugin->name());
}

void NotificationManager::removePlugin(const std::string& name)
{
    plugins_.erase(name);
    spdlog::info("Notification plugin removed: {}", name);
}

QCoro::Task<void> NotificationManager::sendBatchNotifications(
    const QString& message)
{
    std::vector<QCoro::Task<void>> tasks;
    for (const auto& [name, plugin] : plugins_) {
        tasks.push_back(plugin->sendNotification(message));
    }

    for (const auto& task : tasks) {
        try {
            co_await task;
        } catch (const std::exception& e) {
            spdlog::error("Notification failed: {}", e.what());
        }
    }

    co_return;
}
}
