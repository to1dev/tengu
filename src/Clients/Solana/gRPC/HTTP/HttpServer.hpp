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
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

#include "../Core/ConfigManager.hpp"

namespace solana {

class HttpServer {
public:
    explicit HttpServer(const ConfigManager& config);
    ~HttpServer();

    void addRoute(const std::string& path,
        std::function<http::response<http::string_body>(
            const http::request<http::string_body>&, const std::string&,
            const std::map<std::string, std::string>&)>
            handler);
    void start();
    void stop();

private:
    class Session;

    void doAccept();

    void incrementRequestCount()
    {
        std::lock_guard lock(mutex_);
        ++requestCount_;
    }

    const ConfigManager& config_;
    net::io_context ioc_;
    tcp::acceptor acceptor_;
    std::map<std::string,
        std::function<http::response<http::string_body>(
            const http::request<http::string_body>&, const std::string&,
            const std::map<std::string, std::string>&)>>
        routes_;
    uint64_t requestCount_ = 0;
    std::mutex mutex_;
    bool running_ = false;
    std::vector<std::jthread> threads_;
};
}
