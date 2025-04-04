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
    config_ = toml::parse_file(configFile);
    if (sodium_init() < 0) {
        throw std::runtime_error("Libsodium initialization failed");
    }

    crypto_secretbox_keygen(key_);
    randombytes_buf(nonce_, sizeof(nonce_));
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
    return config_["db_path"].value_or("./solana_geyser_db");
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

std::string ConfigManager::encrypt(const std::string& data) const
{
    std::vector<unsigned char> ciphertext(
        crypto_secretbox_MACBYTES + data.size());
    if (crypto_secretbox_easy(ciphertext.data(),
            reinterpret_cast<const unsigned char*>(data.data()), data.size(),
            nonce_, key_)
        != 0) {
        throw std::runtime_error("Encryption failed");
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
        return "";
    }
    return std::string(plaintext.begin(), plaintext.end());
}
}
