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

#include "NotificationManager.hpp"

#include <QEventLoop>
#include <QNetworkReply>

#include "../Utils/Logger.hpp"

namespace solana {

class TelegramPlugin : public NotificationPlugin {
public:
    TelegramPlugin(const QString& botToken, const QString& chatId)
        : botToken_(botToken)
        , chatId_(chatId)
    {
        networkManager_.setTransferTimeout(10000);
    }

    bool sendNotification(const QString& message) override
    {
        constexpr int MAX_RETRIES = 3;
        for (int retry = 0; retry < MAX_RETRIES; ++retry) {
            QString url = QString("https://api.telegram.org/bot%1/sendMessage")
                              .arg(botToken_);
            QNetworkRequest request { QUrl(url) };
            request.setHeader(
                QNetworkRequest::ContentTypeHeader, "application/json");

            json json;
            json["chat_id"] = chatId_.toStdString();
            json["text"] = message.toStdString();
            json["parse_mode"] = "HTML";

            QEventLoop loop;
            auto reply = std::unique_ptr<QNetworkReply>(networkManager_.post(
                request, QByteArray::fromStdString(json.dump())));

            QObject::connect(reply.get(), &QNetworkReply::finished, &loop,
                &QEventLoop::quit);
            loop.exec();

            bool success = reply->error() == QNetworkReply::NoError;

            if (success) {
                ++successCount_;
                return true;
            }

            Logger::getLogger()->error(
                "Telegram notification attempt {}/{} failed", retry + 1,
                MAX_RETRIES);

            std::this_thread::sleep_for(
                std::chrono::milliseconds(1000 * (retry + 1)));
        }

        ++failureCount_;
        return false;
    }

    std::string name() const override
    {
        return "telegram";
    }

    uint64_t getSuccessCount() const override
    {
        return successCount_;
    }

    uint64_t getFailureCount() const override
    {
        return failureCount_;
    }

private:
    QString botToken_;
    QString chatId_;
    uint64_t successCount_ { 0 };
    uint64_t failureCount_ { 0 };
    QNetworkAccessManager networkManager_;
};

class DiscordPlugin : public NotificationPlugin {
public:
    explicit DiscordPlugin(const QString& webhookUrl)
        : webhookUrl_(webhookUrl)
    {
        networkManager_.setTransferTimeout(10000);
    }

    bool sendNotification(const QString& message) override
    {
        constexpr int MAX_RETRIES = 3;
        for (int retry = 0; retry < MAX_RETRIES; ++retry) {
            QNetworkRequest request { QUrl(webhookUrl_) };
            request.setHeader(
                QNetworkRequest::ContentTypeHeader, "application/json");

            json json;
            json["content"] = message.toStdString();

            QEventLoop loop;
            auto reply = std::unique_ptr<QNetworkReply>(networkManager_.post(
                request, QByteArray::fromStdString(json.dump())));

            QObject::connect(reply.get(), &QNetworkReply::finished, &loop,
                &QEventLoop::quit);
            loop.exec();

            bool success = reply->error() == QNetworkReply::NoError;

            if (success) {
                ++successCount_;
                return true;
            }

            Logger::getLogger()->error(
                "Discord notification attempt {}/{} failed", retry + 1,
                MAX_RETRIES);

            std::this_thread::sleep_for(
                std::chrono::milliseconds(1000 * (retry + 1)));
        }

        ++failureCount_;
        return false;
    }

    std::string name() const override
    {
        return "discord";
    }

    uint64_t getSuccessCount() const override
    {
        return successCount_;
    }

    uint64_t getFailureCount() const override
    {
        return failureCount_;
    }

private:
    QString webhookUrl_;
    uint64_t successCount_ { 0 };
    uint64_t failureCount_ { 0 };
    QNetworkAccessManager networkManager_;
};

class NotificationWorker {
public:
    NotificationWorker()
    {
        worker_ = std::jthread([this](std::stop_token stoken) { run(stoken); });
    }

    ~NotificationWorker()
    {
        stop();
    }

    void enqueueNotification(const QString& message, NotificationPlugin* plugin)
    {
        if (!plugin)
            return;
        std::lock_guard lock(mutex_);
        tasks_.push({ message, plugin });
        condition_.notify_one();
    }

private:
    struct NotificationTask {
        QString message;
        NotificationPlugin* plugin;
    };

    void stop()
    {
        {
            std::lock_guard lock(mutex_);
            shouldRun_ = false;
            condition_.notify_all();
        }
        worker_.request_stop();
        worker_.join();
    }

    void run(std::stop_token stoken)
    {
        Logger::getLogger()->info("Notification worker started");
        while (!stoken.stop_requested()) {
            NotificationTask task;
            {
                std::unique_lock lock(mutex_);
                condition_.wait(lock, [this, &stoken] {
                    return !tasks_.empty() || !shouldRun_
                        || stoken.stop_requested();
                });

                if (!shouldRun_ && tasks_.empty())
                    break;

                task = std::move(tasks_.front());
                tasks_.pop();
            }

            bool success = task.plugin->sendNotification(task.message);
            // Signal emission handled in NotificationManager
        }

        Logger::getLogger()->info("Notification worker stopped");
    }

    std::jthread worker_;
    std::queue<NotificationTask> tasks_;
    std::mutex mutex_;
    std::condition_variable condition_;
    bool shouldRun_ { true };
};

NotificationManager::NotificationManager(
    const ConfigManager& config, QObject* parent)
    : QObject(parent)
    , config_(config)
    , notificationsEnabled_(true)
{
    if (auto telegram = config_.getTelegramConfig()) {
        addPlugin(std::make_unique<TelegramPlugin>(
            QString::fromStdString(telegram->first),
            QString::fromStdString(telegram->second)));
    }

    if (auto discord = config_.getDiscordConfig()) {
        addPlugin(
            std::make_unique<DiscordPlugin>(QString::fromStdString(*discord)));
    }

    worker_ = std::make_unique<NotificationWorker>();
}

NotificationManager::~NotificationManager() = default;

void NotificationManager::addPlugin(std::unique_ptr<NotificationPlugin> plugin)
{
    std::lock_guard lock(mutex_);
    plugins_[plugin->name()] = std::move(plugin);
    Logger::getLogger()->info("Notification plugin added: {}", plugin->name());
    Q_EMIT pluginAdded(QString::fromStdString(plugin->name()));
}

void NotificationManager::removePlugin(const std::string& name)
{
    std::lock_guard lock(mutex_);
    plugins_.erase(name);
    Logger::getLogger()->info("Notification plugin removed: {}", name);
    Q_EMIT pluginRemoved(QString::fromStdString(name));
}

void NotificationManager::sendBatchNotifications(const QString& message)
{
    if (!notificationsEnabled_)
        return;

    std::lock_guard lock(mutex_);
    for (const auto& [name, plugin] : plugins_) {
        worker_->enqueueNotification(message, plugin.get());
    }
}

void NotificationManager::setNotificationsEnabled(bool enabled)
{
    notificationsEnabled_ = enabled;
    Logger::getLogger()->info(
        "Notifications {}", enabled ? "enabled" : "disabled");
}

json NotificationManager::getNotificationStats() const
{
    json stats;
    std::lock_guard lock(mutex_);
    for (const auto& [name, plugin] : plugins_) {
        stats[name] = { { "success", plugin->getSuccessCount() },
            { "failure", plugin->getFailureCount() } };
    }
    return stats;
}
}
