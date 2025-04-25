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

#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include <QNetworkAccessManager>
#include <QObject>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "ConfigManager.hpp"

namespace solana {

class NotificationWorker;

class NotificationPlugin {
public:
    virtual ~NotificationPlugin() = default;
    virtual bool sendNotification(const QString& message) = 0;
    virtual std::string name() const = 0;
    virtual uint64_t getSuccessCount() const = 0;
    virtual uint64_t getFailureCount() const = 0;
};

class NotificationManager : public QObject {
    Q_OBJECT

public:
    explicit NotificationManager(
        const ConfigManager& config, QObject* parent = nullptr);
    ~NotificationManager();

    void addPlugin(std::unique_ptr<NotificationPlugin> plugin);
    void removePlugin(const std::string& name);
    void sendBatchNotifications(const QString& message);
    void setNotificationsEnabled(bool enabled);
    json getNotificationStats() const;

Q_SIGNALS:
    void notificationSent(const QString& pluginName, bool success);
    void notificationError(
        const QString& pluginName, const QString& errorMessage);
    void pluginAdded(const QString& name);
    void pluginRemoved(const QString& name);

private:
    std::map<std::string, std::unique_ptr<NotificationPlugin>> plugins_;
    std::unique_ptr<NotificationWorker> worker_;
    const ConfigManager& config_;
    mutable std::mutex mutex_;
    bool notificationsEnabled_;
};
}
