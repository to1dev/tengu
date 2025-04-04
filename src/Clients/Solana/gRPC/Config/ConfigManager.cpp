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

#include "ConfigManager.h"

namespace Daitengu::Clients::Solana::gRPC::Config {

ConfigManager::ConfigManager()
    : QObject(nullptr)
{
}

ConfigManager::~ConfigManager() = default;

ConfigManager& ConfigManager::instance()
{
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::loadConfig(const std::string& path)
{
    return false;
}

bool ConfigManager::reload()
{
    return false;
}

std::vector<std::string> ConfigManager::getEnabledChains() const
{
    return std::vector<std::string>();
}

std::string ConfigManager::getSolanaEndpoint() const
{
    return std::string();
}

std::unordered_set<std::string> ConfigManager::getSmartWallets() const
{
    return std::unordered_set<std::string>();
}

std::vector<std::string> ConfigManager::getSolanaFilters() const
{
    return std::vector<std::string>();
}

std::string ConfigManager::getEthereumEndpoint() const
{
    return std::string();
}

std::vector<std::string> ConfigManager::getEthereumFilters() const
{
    return std::vector<std::string>();
}

std::string ConfigManager::getDbPath() const
{
    return std::string();
}

bool ConfigManager::isNotificationEnabled() const
{
    return false;
}

std::string ConfigManager::getTelegramToken() const
{
    return std::string();
}

std::string ConfigManager::getTelegramChatId() const
{
    return std::string();
}

double ConfigManager::getMinNotificationAmount() const
{
    return 0.0;
}

int ConfigManager::getMaxReconnectAttempts() const
{
    return 0;
}

bool ConfigManager::isDebugModeEnabled() const
{
    return false;
}
}
