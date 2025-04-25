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

#include "ConfigManager.hpp"

#include <stdexcept>
#include <system_error>

#include "Consts.h"

#include "Utils/PathUtils.hpp"

#include "../Utils/Logger.hpp"

using namespace Daitengu::Core;
using namespace Daitengu::Utils;

namespace solana {

ConfigManager::ConfigManager(const std::string& configFile)
    : configFile_(configFile)
    , dataPath_(PathUtils::getAppDataPath(COMPANY) / NAME)
{
    if (sodium_init() < 0) {
        throw std::runtime_error("Libsodium initialization failed");
    }

    initializeKeyAndNonce();
    loadConfig();
}

ConfigManager::~ConfigManager()
{
    sodium_memzero(key_, sizeof(key_));
    sodium_memzero(nonce_, sizeof(nonce_));
}

void ConfigManager::loadConfig()
{
    fs::path configPath = PathUtils::toAbsolutePath(configFile_);
    if (!PathUtils::exists(configPath)) {
        throw std::runtime_error(
            "Config file not found: " + configPath.string());
    }

    try {
        std::lock_guard lock(mutex_);
        config_ = toml::parse_file(configPath.string());
        Logger::getLogger()->info(
            "Config loaded from: {}", configPath.string());
    } catch (const toml::parse_error& e) {
        Logger::getLogger()->error("Failed to parse config file {}: {}",
            configPath.string(), e.what());
        throw std::runtime_error(
            "Config parse error: " + std::string(e.what()));
    }
}

std::vector<ConfigManager::DataSourceConfig>
ConfigManager::getDataSources() const
{
    std::lock_guard lock(mutex_);
    std::vector<DataSourceConfig> result;
    if (auto sources = config_["data_sources"].as_array()) {
        for (const auto& src : *sources) {
            if (auto table = src.as_table()) {
                DataSourceConfig cfg;
                cfg.address = table->get("address")->value_or<std::string>("");
                cfg.type = table->get("type")->value_or<std::string>("geyser");
                if (!cfg.address.empty()) {
                    result.push_back(cfg);
                }
            }
        }
    }

    return result;
}

std::string ConfigManager::getDbPath() const
{
    std::lock_guard lock(mutex_);
    std::string dbPath = config_["db_path"].value_or("solana_monitor_db");

    return (dataPath_ / dbPath).string();
}

std::optional<std::pair<std::string, std::string>>
ConfigManager::getTelegramConfig() const
{
    std::lock_guard lock(mutex_);
    if (auto telegram = config_["telegram"].as_table()) {
        auto botToken = decrypt(telegram->get("bot_token")->value_or(""));
        auto chatId = telegram->get("chat_id")->value_or<std::string>("");
        if (!botToken.empty() && !chatId.empty()) {
            return std::make_pair(botToken, chatId);
        }
    }

    return std::nullopt;
}

std::optional<std::string> ConfigManager::getDiscordConfig() const
{
    std::lock_guard lock(mutex_);
    if (auto discord = config_["discord"].as_table()) {
        auto webhookUrl = decrypt(discord->get("webhook_url")->value_or(""));
        if (!webhookUrl.empty()) {
            return webhookUrl;
        }
    }

    return std::nullopt;
}

int ConfigManager::getHttpPort() const
{
    std::lock_guard lock(mutex_);
    return config_["http_port"].value_or(8080);
}

std::string ConfigManager::getLogLevel() const
{
    std::lock_guard lock(mutex_);
    return config_["log_level"].value_or("info");
}

int ConfigManager::getMaxConcurrentFilters() const
{
    std::lock_guard lock(mutex_);
    return config_["max_concurrent_filters"].value_or(
        std::thread::hardware_concurrency() / 2);
}

int ConfigManager::getHealthCheckIntervalSeconds() const
{
    std::lock_guard lock(mutex_);
    return config_["health_check_interval_seconds"].value_or(60);
}

bool ConfigManager::reload()
{
    try {
        loadConfig();
        return true;
    } catch (const std::exception& e) {
        Logger::getLogger()->error("Failed to reload config: {}", e.what());
        return false;
    }
}

void ConfigManager::initializeKeyAndNonce()
{
    fs::create_directories(dataPath_);
    fs::path keyFile = dataPath_ / "secret.key";
    std::ifstream file(keyFile, std::ios::binary);
    if (file.is_open()) {
        file.read(reinterpret_cast<char*>(key_), sizeof(key_));
        file.read(reinterpret_cast<char*>(nonce_), sizeof(nonce_));
        file.close();
        if (file.gcount() != sizeof(nonce_)) {
            Logger::getLogger()->error("Corrupt key file, regenerating");
            generateKeyAndNonce(keyFile);
        } else {
            Logger::getLogger()->info(
                "Loaded key and nonce from {}", keyFile.string());
        }
    } else {
        generateKeyAndNonce(keyFile);
    }
}

void ConfigManager::generateKeyAndNonce(const fs::path& keyFile)
{
    crypto_secretbox_keygen(key_);
    randombytes_buf(nonce_, sizeof(nonce_));
    std::error_code ec;
    fs::create_directories(dataPath_, ec);
    if (ec) {
        throw std::runtime_error(
            "Cannot create data directory: " + ec.message());
    }

    std::ofstream outFile(keyFile, std::ios::binary);
    if (!outFile.is_open()) {
        throw std::runtime_error("Cannot write key file");
    }

    outFile.write(reinterpret_cast<const char*>(key_), sizeof(key_));
    outFile.write(reinterpret_cast<const char*>(nonce_), sizeof(nonce_));
    outFile.close();
    Logger::getLogger()->info(
        "Generated key and nonce to {}", keyFile.string());
}

std::string ConfigManager::encrypt(const std::string& data) const
{
    if (data.empty())
        return "";

    std::vector<unsigned char> ciphertext(
        crypto_secretbox_MACBYTES + data.size());
    if (crypto_secretbox_easy(ciphertext.data(),
            reinterpret_cast<const unsigned char*>(data.data()), data.size(),
            nonce_, key_)
        != 0) {
        Logger::getLogger()->error("Encryption failed");
        return "";
    }

    std::string result;
    result.reserve(ciphertext.size() * 2);
    for (unsigned char c : ciphertext) {
        static const char hex[] = "0123456789abcdef";
        result.push_back(hex[c >> 4]);
        result.push_back(hex[c & 0xF]);
    }

    return result;
}

std::string ConfigManager::decrypt(const std::string& hexData) const
{
    if (hexData.empty() || hexData.size() % 2 != 0)
        return "";

    std::vector<unsigned char> data(hexData.size() / 2);
    for (size_t i = 0; i < data.size(); ++i) {
        unsigned char byte = 0;
        for (int j = 0; j < 2; ++j) {
            char c = hexData[i * 2 + j];
            byte <<= 4;
            if (c >= '0' && c <= '9')
                byte |= c - '0';
            else if (c >= 'a' && c <= 'f')
                byte |= c - 'a' + 10;
            else if (c >= 'A' && c <= 'F')
                byte |= c - 'A' + 10;
            else
                return "";
        }
        data[i] = byte;
    }

    if (data.size() < crypto_secretbox_MACBYTES)
        return "";

    std::vector<unsigned char> plaintext(
        data.size() - crypto_secretbox_MACBYTES);

    if (crypto_secretbox_open_easy(
            plaintext.data(), data.data(), data.size(), nonce_, key_)
        != 0) {
        Logger::getLogger()->warn("Decryption failed");
        return "";
    }

    return std::string(plaintext.begin(), plaintext.end());
}
}
