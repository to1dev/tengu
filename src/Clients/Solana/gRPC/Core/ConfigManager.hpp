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

#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include <sodium.h>

#include <toml.hpp>

namespace solana {

class ConfigManager {
public:
    explicit ConfigManager(const std::string& configFile);
    ~ConfigManager();

    struct DataSourceConfig {
        std::string address;
        std::string type;
    };

    std::vector<DataSourceConfig> getDataSources() const;
    std::string getDbPath() const;
    std::optional<std::pair<std::string, std::string>>
    getTelegramConfig() const;
    std::optional<std::string> getDiscordConfig() const;
    int getHttpPort() const;
    std::string getLogLevel() const;
    int getMaxConcurrentFilters() const;
    int getHealthCheckIntervalSeconds() const;

    bool reload();
    std::string encrypt(const std::string& data) const;
    std::string decrypt(const std::string& data) const;

private:
    void loadConfig();
    void initializeKeyAndNonce();
    void generateKeyAndNonce(const std::filesystem::path& keyFile);

    std::string configFile_;
    toml::table config_;
    mutable std::mutex mutex_;
    unsigned char key_[crypto_secretbox_KEYBYTES];
    unsigned char nonce_[crypto_secretbox_NONCEBYTES];
    std::filesystem::path dataPath_;
};
}
