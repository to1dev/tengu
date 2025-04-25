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

#include "HttpServer.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "../Utils/Logger.hpp"

namespace solana {

class HttpServer::Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, HttpServer& server)
        : socket_(std::move(socket))
        , server_(server)
    {
    }

    void start()
    {
        doRead();
    }

private:
    void doRead()
    {
        req_ = {};
        http::async_read(socket_, buffer_, req_,
            beast::bind_front_handler(&Session::onRead, shared_from_this()));
    }

    void onRead(beast::error_code ec, std::size_t)
    {
        if (ec) {
            Logger::getLogger()->error("HTTP read error: {}", ec.message());
            return;
        }

        server_.incrementRequestCount();
        Logger::getLogger()->debug("Received HTTP request: {}", req_.target());

        auto path = req_.target();
        auto query = std::map<std::string, std::string>();
        auto pos = path.find('?');
        if (pos != std::string::npos) {
            auto queryStr = path.substr(pos + 1);
            path = path.substr(0, pos);
            // Parse query parameters
            std::string key, value;
            bool parsingKey = true;
            for (char c : queryStr) {
                if (c == '=') {
                    parsingKey = false;
                } else if (c == '&') {
                    if (!key.empty())
                        query[key] = value;
                    key.clear();
                    value.clear();
                    parsingKey = true;
                } else {
                    (parsingKey ? key : value) += c;
                }
            }
            if (!key.empty())
                query[key] = value;
        }

        http::response<http::string_body> res { http::status::not_found,
            req_.version() };
        if (server_.routes_.count(path)) {
            res = server_.routes_[path](req_, path, query);
        } else {
            res.set(http::field::content_type, "text/plain");
            res.body() = "Not Found";
        }

        res.prepare_payload();
        http::async_write(socket_, res,
            beast::bind_front_handler(&Session::onWrite, shared_from_this()));
    }

    void onWrite(beast::error_code ec, std::size_t)
    {
        if (ec) {
            Logger::getLogger()->error("HTTP write error: {}", ec.message());
        }
        socket_.shutdown(tcp::socket::shutdown_send, ec);
    }

    tcp::socket socket_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    HttpServer& server_;
};

HttpServer::HttpServer(const ConfigManager& config)
    : config_(config)
    , ioc_(1)
    , acceptor_(ioc_)
{
}

HttpServer::~HttpServer()
{
    stop();
}

void HttpServer::addRoute(const std::string& path,
    std::function<http::response<http::string_body>(
        const http::request<http::string_body>&, const std::string&,
        const std::map<std::string, std::string>&)>
        handler)
{
    routes_[path] = [this, handler](const auto& req, const auto& p,
                        const auto& q) { return handler(req, p, q); };
    Logger::getLogger()->info("Added HTTP route: {}", path);
}

void HttpServer::start()
{
    try {
        auto port = config_.getHttpPort();
        auto address = net::ip::make_address("0.0.0.0");
        tcp::endpoint endpoint(address, port);

        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(net::socket_base::reuse_address(true));
        acceptor_.bind(endpoint);
        acceptor_.listen(net::socket_base::max_listen_connections);

        running_ = true;
        doAccept();

        // Start IO context in multiple threads
        auto threadCount = std::max(
            1, static_cast<int>(std::thread::hardware_concurrency() / 2));
        for (int i = 0; i < threadCount; ++i) {
            threads_.emplace_back([this] { ioc_.run(); });
        }

        // Add health check route
        addRoute("/health",
            [this](const http::request<http::string_body>&, const std::string&,
                const std::map<std::string, std::string>&) {
                http::response<http::string_body> res { http::status::ok, 11 };
                res.set(http::field::content_type, "application/json");
                res.body() = nlohmann::json {
                    { "status", "healthy" }, { "requests", requestCount_ }
                }.dump();
                res.prepare_payload();
                return res;
            });

        Logger::getLogger()->info("HTTP server started on port {}", port);
    } catch (const std::exception& e) {
        Logger::getLogger()->error("HTTP server failed to start: {}", e.what());
    }
}

void HttpServer::stop()
{
    if (!running_)
        return;
    running_ = false;
    acceptor_.close();
    ioc_.stop();
    threads_.clear();
    Logger::getLogger()->info("HTTP server stopped");
}

void HttpServer::doAccept()
{
    acceptor_.async_accept(
        ioc_, [this](beast::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::make_shared<Session>(std::move(socket), *this)->start();
            }
            if (running_)
                doAccept();
        });
}
}
