#include <iostream>

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>

using namespace boost::asio;
using tcp = ip::tcp;
using namespace std::literals;

awaitable<void> session(tcp::socket socket)
{
    try {
        char data[1024];
        for (;;) {
            std::size_t n
                = co_await socket.async_read_some(buffer(data), use_awaitable);
            co_await async_write(socket, buffer(data, n), use_awaitable);
        }
    } catch (const boost::system::system_error& e) {
        if (e.code() == boost::asio::error::eof) {
            std::cout << "[Info] Client disconnected gracefully."
                      << std::endl;
        } else {
            std::cout << "[Error] Session ended with exception: " << e.what()
                      << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "[Info] Session ended: " << e.what() << std::endl;
    }
}

awaitable<void> listener(io_context& ctx, unsigned short port)
{
    tcp::acceptor acceptor(ctx, { tcp::v4(), port });
    for (;;) {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        co_spawn(ctx, session(std::move(socket)), detached);
    }
}

int main()
{
    io_context ctx(1);
    co_spawn(ctx, listener(ctx, 9000), detached);
    ctx.run();
}
