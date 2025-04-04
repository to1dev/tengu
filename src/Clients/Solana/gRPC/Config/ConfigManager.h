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
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include <QDebug>
#include <QFileSystemWatcher>
#include <QObject>
#include <QString>

namespace Daitengu::Clients::Solana::gRPC::Config {

class ConfigManager : public QObject {
    Q_OBJECT

public:
    static ConfigManager& instance();

    bool loadConfig(const std::string& path);
    bool reload();

    std::vector<std::string> getEnabledChains() const;

    std::string getSolanaEndpoint() const;
    std::unordered_set<std::string> getSmartWallets() const;
    std::vector<std::string> getSolanaFilters() const;

    std::string getEthereumEndpoint() const;
    std::vector<std::string> getEthereumFilters() const;

    std::string getDbPath() const;

    bool isNotificationEnabled() const;
    std::string getTelegramToken() const;
    std::string getTelegramChatId() const;
    double getMinNotificationAmount() const;

    int getMaxReconnectAttempts() const;
    bool isDebugModeEnabled() const;

Q_SIGNALS:
    void configChanged();

private:
    ConfigManager();
    ~ConfigManager();

    void setupWatcher();

    std::string configPath_;
    std::unique_ptr<void, std::function<void(void*)>> config_;
    QFileSystemWatcher fileWatcher_;
};

}
