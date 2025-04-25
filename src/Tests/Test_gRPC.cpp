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

#include <iostream>
#include <string>

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "Clients/Solana/gRPC/Core/ConfigManager.hpp"
#include "Clients/Solana/gRPC/HTTP/HttpServer.hpp"

using namespace solana;

int main()
{
    try {
        spdlog::info("Hello world");

        ConfigManager config("config.toml");

        spdlog::info(config.getDbPath());
        spdlog::info(config.getHttpPort());

        HttpServer httpServer(config);
        httpServer.addRoute("/stats",
            [&](const auto& req, const auto& path, const auto& query) {
                http::response<http::string_body> res { http::status::ok,
                    req.version() };
                res.set(http::field::content_type, "application/json");
                json stats;
                stats["data_sources"] = "data sources";
                stats["filters"] = "filters";
                stats["notifications"] = "notifications";
                stats["storage"]
                    = { { "total_transactions", "total transactions" },
                          { "total_batches", "total batches" } };
                res.body() = stats.dump();
                res.prepare_payload();
                return res;
            });

        httpServer.addRoute("/recent_dex",
            [&](const auto& req, const auto& path, const auto& query) {
                http::response<http::string_body> res { http::status::ok,
                    req.version() };
                res.set(http::field::content_type, "application/json");
                size_t maxEntries = 10;
                if (query.count("max")) {
                    maxEntries = std::stoul(query.at("max"));
                }
                res.body() = "dexFilter body";
                res.prepare_payload();
                return res;
            });

        httpServer.addRoute("/recent_swaps",
            [&](const auto& req, const auto& path, const auto& query) {
                http::response<http::string_body> res { http::status::ok,
                    req.version() };
                res.set(http::field::content_type, "application/json");
                size_t maxEntries = 10;
                if (query.count("max")) {
                    maxEntries = std::stoul(query.at("max"));
                }
                res.body() = "swapFilter body";
                res.prepare_payload();
                return res;
            });

        httpServer.start();

        spdlog::info("Solana Monitor System running. Press Ctrl+C to stop.");
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (const std::exception& e) {
        spdlog::error("System error: {}", e.what());
        return 1;
    }

    return 0;
}
