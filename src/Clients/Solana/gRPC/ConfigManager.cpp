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

namespace Daitengu::Clients::Solana::gRPC {

ConfigManager::ConfigManager(const std::string& configFile)
{
    fs::path configPath = PathUtils::toAbsolutePath(configFile);
    if (!PathUtils::exists(configPath)) {
        throw std::runtime_error(
            "Config file not found: " + configPath.string());
    }
    config_ = toml::parse_file(configPath.string());
    if (sodium_init() < 0) {
        throw std::runtime_error("Libsodium initialization failed");
    }

    initializeKeyAndNonce();
}

ConfigManager::~ConfigManager()
{
    sodium_memzero(key_, sizeof(key_));
    sodium_memzero(nonce_, sizeof(nonce_));
}

std::vector<std::string> ConfigManager::getGeyserAddresses() const
{
    if (auto addresses = config_["geyser_addresses"].as_array()) {
        std::vector<std::string> result;
        for (const auto& addr : *addresses) {
            if (addr.is_string()) {
                result.push_back(addr.as_string()->get());
            }
        }
        return result;
    }
    return {};
}

std::string ConfigManager::getDbPath() const
{
    return (dataPath_ / "solana_geyser_db").string();
}

std::optional<std::pair<std::string, std::string>>
ConfigManager::getTelegramConfig() const
{
    if (auto telegram = config_["telegram"].as_table()) {
        auto botToken = decrypt(telegram->get("bot_token")->value_or(""));
        auto chatId = telegram->get("chat_id")->value_or("");
        return std::make_pair(botToken, chatId);
    }
    return std::nullopt;
}

std::optional<std::string> ConfigManager::getDiscordConfig() const
{
    if (auto discord = config_["discord"].as_table()) {
        return decrypt(discord->get("webhook_url")->value_or(""));
    }
    return std::nullopt;
}

int ConfigManager::getHttpPort() const
{
    return config_["http_port"].value_or(8080);
}

void ConfigManager::initializeKeyAndNonce()
{
    fs::path keyFile = dataPath_ / "secret.key";
    std::ifstream file(keyFile, std::ios::binary);
    if (file.is_open()) {
        file.read(reinterpret_cast<char*>(key_), sizeof(key_));
        file.read(reinterpret_cast<char*>(nonce_), sizeof(nonce_));
        file.close();
        spdlog::info("Loaded key and nonce from {}", keyFile.string());
    } else {
        crypto_secretbox_keygen(key_);
        randombytes_buf(nonce_, sizeof(nonce_));

        std::ofstream outFile(keyFile, std::ios::binary);
        if (!outFile.is_open()) {
            spdlog::error("Failed to create key file: {}", keyFile.string());
            throw std::runtime_error("Cannot write key file");
        }
        outFile.write(reinterpret_cast<const char*>(key_), sizeof(key_));
        outFile.write(reinterpret_cast<const char*>(nonce_), sizeof(nonce_));
        outFile.close();

#ifdef __unix__
        chmod(keyFile.c_str(), S_IRUSR | S_IWUSR);
#endif
        spdlog::info("Generated and saved key and nonce to {}", keyFile.string());
    }
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
        spdlog::error("Encryption failed");
        return "";
    }
    return std::string(ciphertext.begin(), ciphertext.end());
}

std::string ConfigManager::decrypt(const std::string& data) const
{
    if (data.size() < crypto_secretbox_MACBYTES)
        return "";
    std::vector<unsigned char> plaintext(
        data.size() - crypto_secretbox_MACBYTES);
    if (crypto_secretbox_open_easy(plaintext.data(),
            reinterpret_cast<const unsigned char*>(data.data()), data.size(),
            nonce_, key_)
        != 0) {
        spdlog::warn("Decryption failed");
        return "";
    }
    return std::string(plaintext.begin(), plaintext.end());
}
}
