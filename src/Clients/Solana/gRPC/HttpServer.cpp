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

#include "HttpServer.h"

namespace Daitengu::Clients::Solana::gRPC {

HttpServer::HttpServer(io_context& io_context, unsigned short port)
    : io_context_(io_context)
    , acceptor_(io_context, { tcp::v4(), port })
{
}

void HttpServer::start()
{
    co_spawn(
        io_context_,
        [this]() -> awaitable<void> {
            while (true) {
                auto socket = std::make_shared<tcp::socket>(io_context_);
                co_await acceptor_.async_accept(*socket, use_awaitable);
                co_spawn(io_context_, handleClient(socket), detached);
            }
        },
        detached);
}

awaitable<void> HttpServer::handleClient(std::shared_ptr<tcp::socket> socket)
{
    char buffer[1024];
    auto bytes = co_await socket->async_read_some(
        boost::asio::buffer(buffer), use_awaitable);
    std::string request(buffer, bytes);

    std::string response
        = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
    co_await async_write(*socket, boost::asio::buffer(response), use_awaitable);

    socket->close();
}
}
