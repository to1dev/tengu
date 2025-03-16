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

#include <format>
#include <iostream>
#include <ranges>
#include <unordered_set>

#include <QCoreApplication>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QWebSocket>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "fmt/color.h"
#include "fmt/format.h"

#include "ankerl/unordered_dense.h"

#include "Utils/Dotenv.hpp"
#include "Utils/PathUtils.hpp"
using namespace Daitengu::Utils;

static const ankerl::unordered_dense::map<std::string_view, std::string_view>
    dexMap = {
        { "Pump.fun Amm", "pAMMBay6oceH9fJKBRHGP5D4bD4sWpmSwMn52FMfXEA" },
        { "Saber", "SSwpkEEcbUqx4vtoEByFjSkhKdCT862DNVb52nZg1UZ" },
        { "Meteora", "Eo7WjKq67rjJQSZxS6z3YkapzY3eMj6Xy8X5EQVn5UaB" },
        { "Lifinity V2", "2wT8Yq49kHgDzXuPxZSaeLaH1qbmGXtEyPy64bL7aD3c" },
        { "Bonkswap", "BSwp6bEBihVLdqJRKGgzjcGLHkcTuzmSo1TQkHepzH8p" },
        { "Guacswap", "Gswppe6ERWKpUTXvRPfXdzHhiCyJvLadVvXGfdpBqcE1" },
        { "Token Swap", "SwaPpA9LAaLfeLi3a68M4DjnLqgtticKg6CnyNwgAC8" },
        { "Moonshot", "MoonCVVNZFSYkqNXP6bxHLPL6QQJiMagDL3qcqUQTrG" },
        { "Sanctum", "stkitrT1Uoy18Dk1fTrgPw8W6MVzoCfYoAFT4MLsmhq" },
        { "Token Mill", "JoeaRXgtME3jAoz5WuFXGEndfv4NPH9nBxsLq44hk9J" },
        { "Raydium", "675kPX9MHTjS2zt1qfr1NYHuzeLXfQM9H24wFSUt1Mp8" },
        { "Whirlpool", "whirLbMiicVdio4qvUfM5KAg6Ct8VwpYzGff3uctyCc" },
        { "Raydium CLMM", "CAMMCzo5YL8w4VFF8KVHrK22GGUsp5VTaW7grrKgrWqK" },
        { "Oasis", "9tKE7Mbmj4mxDjWatikzGAtkoWosiiZX9y6J4Hfm2R8H" },
        { "StepN", "Dooar9JkhdZ7J3LHN3A7YCuoGRUggXhQaG4kijfLGU2j" },
        { "Invariant", "HyaB3W9q6XdA5xwpU4XnSZV94htfmbmqJXZcEbRaJutt" },
        { "Raydium CP", "CPMMoo8L3F4NbTegBCKVNunggL7H1ZpdTHKxQB5qKP1C" },
        { "Saros", "SSwapUtytfBdBn1b9NUGG6foMVPtcWgpRU32HToDUZr" },
        { "Aldrin V2", "CURVGoZn8zycx6FXwwevgBTB2gVvdbGTEpvMJDbgs2t4" },
        { "ZeroFi", "ZERor4xhbUycZ6gb9ntrhqscUcZmAbQDjEAtCf4hbZY" },
        { "Daos.fun", "5jnapfrAN47UYkLkEf7HnprPPBCQLvkYWGZDeKkaP5hv" },
        { "Stabble Stable Swap",
            "swapNyd8XiQwJ6ianp9snpu4brUqFxadzvHebnAXjJZ" },
        { "DexLab", "DSwpgjMvXhtGn6BsbqmacdBZyfLj6jSWf3HJpdJtmg6N" },
        { "Pump.fun", "6EF8rrecthR5Dkzon8Nwu78hRvfCKubJ14M5uBEwF6P" },
        { "Solayer", "endoLNCKTqDn8gSVnN2hDdpgACUPWHZTwoYnnMybpAT" },
        { "Virtuals", "5U3EU2ubXtK84QcRjWVmYt9RaDyA8gKxdUrPFXmZyaki" },
        { "Mercurial", "MERLuDFBMmsHnsBPZw2sDQZHvXFMwp8EdjudcU2HKky" },
        { "FluxBeam", "FLUXubRmkEi2q6K3Y9kBPg9248ggaZVsoSFhtJHSrm1X" },
        { "Orca V2", "9W959DqEETiGZocYWCQPaJ6sBmUzgfxXfqGeTEdp3aQP" },
        { "OpenBook V2", "opnb2LAfJYbRMAHHvqjCwQxanZn7ReEHp1k81EohpZb" },
        { "SolFi", "SoLFiHG9TfgtdUXUjWAxi3LtvYuFyDLVhBWxdMZxyCe" },
        { "Penguin", "PSwapMdSai8tjrEXcxFeQth87xC4rRsa4VA5mhGhXkP" },
        { "Helium Network", "treaf4wWBBty3fHdyBpo35Mz84M8k3heKXmjmi9vFt5" },
        { "Stabble Weighted Swap",
            "swapFpHZwjELNnjvThjajtiVmkz3yPQEHjLtka2fwHW" },
        { "Crema", "CLMM9tUoggJu2wagPkkqs9eFG4BWhVBZWkP1qv3Sp7tR" },
        { "Saber (Decimals)", "DecZY86MU5Gj7kppfUCEmd4LbXXuyZH1yHaP2NTqdiZB" },
        { "Perena", "NUMERUNsFCP3kuNmWZuXtm1AaQCPj9uw6Guv2Ekoi5P" },
        { "Obric V2", "obriQD1zbpyLz95G5n7nJe6a4DPjpFwa5XYPoNm113y" },
        { "1DEX", "DEXYosS6oEGvk8uCDayvwEZz4qEyDJRf9nFgYCaqPMTm" },
        { "Sanctum Infinity", "5ocnV1qiCgaQR8Jb8xWnVbApfaygJ8tNoZfgPwsgx9kx" },
        { "Openbook", "srmqPvymJeFKQ4zGQed1GFppgkRHL9kaELCbyksJtPX" },
        { "Perps", "PERPHjGBqRHArX4DySjwM6UJHiR3sWAatqfdBS2qQJu" },
        { "Cropper", "H8W3ctz92svYg6mkn1UtGfu2aQr2fnUFHM1RhScEtQDt" },
        { "Phoenix", "PhoeNiXZ8ByJGLkxNfZRnkUfjvmuYqLR89jjFHGqdXY" },
        { "Meteora DLMM", "LBUZKhRxPF3XUpBCjp4YzTKgLccjZhTSDM9YuVaPwxo" },
        { "Orca V1", "DjVE6JNiYqPL2QXyCUUh8rNjHrbz9hXHNYt99MQ59qw1" },
        { "Aldrin", "AMM55ShdkoGRB5jVYPjWziwk8m5MpwyDgsMWHaMSQWH6" },
    };

class SolanaClient : public QObject {
    Q_OBJECT

public:
    explicit SolanaClient(const QUrl& url, QObject* parent = nullptr)
        : QObject(parent)
        , m_url(url)
    {
        m_monitoredAddresses
            = { "2j3MGgjTZnf5woD1dV9XScaSy5SxPeKh5eTTzcpZ142z" };

        // 连接信号到槽函数
        connect(&m_webSocket, &QWebSocket::connected, this,
            &SolanaClient::onConnected);
        connect(&m_webSocket, &QWebSocket::disconnected, this,
            &SolanaClient::onDisconnected);
        connect(&m_webSocket, &QWebSocket::textMessageReceived, this,
            &SolanaClient::onTextMessageReceived);

        // 连接心跳定时器，每60秒触发一次
        connect(&m_heartbeatTimer, &QTimer::timeout, this,
            &SolanaClient::sendHeartbeat);

        // 建立连接（注意使用 wss:// 表示 SSL 连接）
        m_webSocket.open(url);
    }

private Q_SLOTS:

    // 连接建立成功后发送订阅消息
    void onConnected()
    {
        qDebug() << "WebSocket connected";

        json params = json::array();
        params.push_back("all");

        json subparam;
        subparam["commitment"] = "finalized";
        params.push_back(subparam);

        json req;
        req["jsonrpc"] = "2.0";
        req["id"] = 1;
        req["method"] = "logsSubscribe";
        req["params"] = params;

        QString subscriptionMessage = QString::fromStdString(req.dump());

        // 发送订阅消息
        m_webSocket.sendTextMessage(subscriptionMessage);

        // 启动心跳定时器（60000 毫秒 = 1 分钟）
        m_heartbeatTimer.start(60000);
    }

    // 处理接收到的文本消息
    void onTextMessageReceived(const QString& message)
    {
        try {
            auto json = json::parse(message.toStdString());
            if (json.contains("params")) {
                auto params = json["params"];
                if (params.contains("result")) {
                    auto result = params["result"];
                    if (result.contains("value")) {
                        auto value = result["value"];
                        if (value.contains("err") && !value["err"].is_null()) {
                            return;
                        }
                        if (value.contains("logs")) {
                            auto logs = value["logs"];

                            std::string foundDexName;
                            bool relevant = std::ranges::any_of(
                                logs, [&foundDexName](const auto& logEntry) {
                                    std::string logLine
                                        = logEntry.template get<std::string>();
                                    for (const auto& [dexName, dexAddress] :
                                        dexMap) {
                                        if (logLine.find(dexAddress)
                                            != std::string::npos) {
                                            foundDexName = dexName;
                                            return true;
                                        }
                                    }
                                    return false;
                                });

                            if (relevant) {
                                if (value.contains("signature")) {
                                    std::string signature = value["signature"];
                                    fmt::print("{} transaction found: {}\n",
                                        fmt::styled(foundDexName,
                                            fmt::fg(fmt::color::green)),
                                        fmt::styled(signature,
                                            fmt::fg(fmt::color::blue)));
                                } else {
                                    qDebug() << "No signature";
                                }
                            }
                        }
                    } else {
                        qDebug() << "No value";
                    }
                } else {
                    qDebug() << "No result";
                }
            } else {
                qDebug() << "No params";
            }
        } catch (std::exception& e) {
            qDebug() << "Failed to parse Json: " << e.what();
        }
    }

    // 当 WebSocket 断开时，启动延时重连
    void onDisconnected()
    {
        qDebug() << "WebSocket disconnected";
        m_heartbeatTimer.stop();
        // 延时 5000 毫秒（5秒）后调用 reconnect()
        QTimer::singleShot(5000, this, &SolanaClient::reconnect);
    }

    void reconnect()
    {
        qDebug() << "Reconnecting...";
        m_webSocket.open(m_url);
    }

    // 心跳函数，每隔1分钟发送一次心跳消息
    void sendHeartbeat()
    {
        qDebug() << "Sending heartbeat";
        // 这里使用文本消息发送心跳，也可以考虑使用 QWebSocket::ping()
        m_webSocket.ping("ping");
    }

private:
    QWebSocket m_webSocket;
    QUrl m_url;
    QTimer m_heartbeatTimer;

    ankerl::unordered_dense::set<std::string> m_monitoredAddresses;
};

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    std::string host = "wss://solana-rpc.publicnode.com";
    auto currentPath = PathUtils::getExecutableDir();
    try {
        auto& parser = DotEnv::getInstance();
        parser.load((currentPath / ".env").string());
        host = parser.getOrDefault("WSS_HOST", host);
    } catch (const DotEnvException& e) {
        std::cerr << "Configuration error: " << e.what() << std::endl;
    }

    QUrl url(QString::fromStdString(host));
    SolanaClient client(url);

    return a.exec();
}

#include "main.moc"
