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

#include <sodium.h>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "fmt/color.h"
#include "fmt/format.h"

#include "ankerl/unordered_dense.h"

#include "Utils/Base58.hpp"
#include "Utils/Borsh.hpp"
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

namespace borsh {
struct SwapEvent {
    PublicKey poolState;
    PublicKey sender;
    PublicKey tokenAccount0;
    PublicKey tokenAccount1;
    std::uint64_t amount0;
    std::uint64_t transferFee0;
    std::uint64_t amount1;
    std::uint64_t transferFee1;
    bool zeroForOne;
    U128 sqrtPriceX64;
    U128 liquidity;
    std::int32_t tick;
};

template <> struct Deserializer<SwapEvent> {
    static SwapEvent deserialize(
        const std::vector<std::uint8_t>& data, size_t& pos)
    {
        SwapEvent evt {};
        evt.poolState = Deserializer<PublicKey>::deserialize(data, pos);
        evt.sender = Deserializer<PublicKey>::deserialize(data, pos);
        evt.tokenAccount0 = Deserializer<PublicKey>::deserialize(data, pos);
        evt.tokenAccount1 = Deserializer<PublicKey>::deserialize(data, pos);
        evt.amount0 = Deserializer<std::uint64_t>::deserialize(data, pos);
        evt.transferFee0 = Deserializer<std::uint64_t>::deserialize(data, pos);
        evt.amount1 = Deserializer<std::uint64_t>::deserialize(data, pos);
        evt.transferFee1 = Deserializer<std::uint64_t>::deserialize(data, pos);
        evt.zeroForOne = Deserializer<bool>::deserialize(data, pos);
        evt.sqrtPriceX64 = Deserializer<U128>::deserialize(data, pos);
        evt.liquidity = Deserializer<U128>::deserialize(data, pos);
        evt.tick = Deserializer<std::int32_t>::deserialize(data, pos);
        return evt;
    }
};
}

inline std::string base64Decode(const std::string& input)
{
    size_t decodedLength = input.size();
    std::vector<unsigned char> decoded(decodedLength);

    if (sodium_base642bin(decoded.data(), decoded.size(), input.data(),
            input.size(), nullptr, &decodedLength, nullptr,
            sodium_base64_VARIANT_ORIGINAL)
        != 0) {
        throw std::runtime_error("Base64 decoding failed");
    }

    return std::string(reinterpret_cast<char*>(decoded.data()), decodedLength);
}

inline std::vector<std::uint8_t> toBytes(const std::string& binStr)
{
    return std::vector<std::uint8_t>(binStr.begin(), binStr.end());
}

json parse_create_instruction(const std::string& data)
{
    if (data.size() < 8) {
        return json();
    }

    size_t offset = 8;
    nlohmann::json parsed_data;

    std::vector<std::pair<std::string, std::string>> fields = {
        { "name", "string" },
        { "symbol", "string" },
        { "uri", "string" },
        { "mint", "publicKey" },
        { "bondingCurve", "publicKey" },
        { "user", "publicKey" },
    };

    try {
        for (const auto& [field_name, field_type] : fields) {
            if (field_type == "string") {
                if (offset + 4 > data.size()) {
                    return json();
                }

                uint32_t length = 0;
                memcpy(&length, data.data() + offset, sizeof(length));
                offset += 4;

                if (offset + length > data.size()) {
                    return json();
                }

                std::string value(
                    data.data() + offset, data.data() + offset + length);
                offset += length;

                parsed_data[field_name] = value;
            } else if (field_type == "publicKey") {
                if (offset + 32 > data.size()) {
                    return json();
                }

                std::vector<unsigned char> key_bytes(
                    reinterpret_cast<const unsigned char*>(
                        data.data() + offset),
                    reinterpret_cast<const unsigned char*>(
                        data.data() + offset + 32));
                offset += 32;

                std::string value = EncodeBase58(key_bytes);

                parsed_data[field_name] = value;
            }
        }
        return parsed_data;
    } catch (const std::exception&) {
        return json();
    }
}

static inline uint64_t read_u64(const std::string& data, size_t& offset)
{
    if (offset + 8 > data.size()) {
        throw std::runtime_error("read_u64: out of range");
    }

    uint64_t val = 0;
    std::memcpy(&val, data.data() + offset, sizeof(val));
    offset += 8;

    return val;
}

static inline std::string read_public_key_base58(
    const std::string& data, size_t& offset)
{
    if (offset + 32 > data.size()) {
        throw std::runtime_error("read_public_key_base58: out of range");
    }

    std::vector<unsigned char> key_bytes(
        reinterpret_cast<const unsigned char*>(data.data() + offset),
        reinterpret_cast<const unsigned char*>(data.data() + offset + 32));
    offset += 32;

    return EncodeBase58(key_bytes);
}

static inline bool read_bool(const std::string& data, size_t& offset)
{
    if (offset + 1 > data.size()) {
        throw std::runtime_error("read_bool: out of range");
    }
    uint8_t b = static_cast<uint8_t>(data[offset]);
    offset += 1;
    return (b != 0);
}

static inline int32_t read_i32(const std::string& data, size_t& offset)
{
    if (offset + 4 > data.size()) {
        throw std::runtime_error("read_i32: out of range");
    }
    int32_t val = 0;
    std::memcpy(&val, data.data() + offset, sizeof(val));
    offset += 4;
    return val;
}

static inline std::string read_u128_as_hex(
    const std::string& data, size_t& offset)
{
    if (offset + 16 > data.size()) {
        throw std::runtime_error("read_u128_as_hex: out of range");
    }

    const unsigned char* ptr
        = reinterpret_cast<const unsigned char*>(data.data() + offset);
    offset += 16;

    static const char* hexDigits = "0123456789abcdef";

    std::string hexStr;
    hexStr.reserve(32);
    for (int i = 0; i < 16; i++) {
        unsigned char c = ptr[i];
        // hi
        hexStr.push_back(hexDigits[c >> 4]);
        // lo
        hexStr.push_back(hexDigits[c & 0x0f]);
    }

    // return "0x" + hexStr;
    return hexStr;
}

json parse_swap_event(const std::string& data)
{
    if (data.size() < 8 + 32 * 4 + 8 * 4 + 1 + 16 * 2 + 4) {
        return json();
    }

    /*if (data.size() < 8) {
        return json();
    }*/

    size_t offset = 8;

    try {
        json result;

        {
            std::string v = read_public_key_base58(data, offset);
            result["poolState"] = v;
        }

        {
            std::string v = read_public_key_base58(data, offset);
            result["sender"] = v;
        }

        {
            std::string v = read_public_key_base58(data, offset);
            result["tokenAccount0"] = v;
        }

        {
            std::string v = read_public_key_base58(data, offset);
            result["tokenAccount1"] = v;
        }

        {
            uint64_t v = read_u64(data, offset);
            result["amount0"] = v;
        }

        {
            uint64_t v = read_u64(data, offset);
            result["transferFee0"] = v;
        }

        {
            uint64_t v = read_u64(data, offset);
            result["amount1"] = v;
        }

        {
            uint64_t v = read_u64(data, offset);
            result["transferFee1"] = v;
        }

        {
            bool v = read_bool(data, offset);
            result["zeroForOne"] = v;
        }

        {
            std::string hexStr = read_u128_as_hex(data, offset);
            result["sqrtPriceX64"] = hexStr;
        }

        {
            std::string hexStr = read_u128_as_hex(data, offset);
            result["liquidity"] = hexStr;
        }

        {
            int32_t v = read_i32(data, offset);
            result["tick"] = v;
        }

        return result;
    } catch (const std::exception& e) {
        std::cerr << "parse_swap_event error: " << e.what() << std::endl;
        return json();
    }
}

class SolanaClient : public QObject {
    Q_OBJECT

public:
    explicit SolanaClient(const QUrl& url, QObject* parent = nullptr)
        : QObject(parent)
        , m_url(url)
    {
        m_monitoredAddresses
            = { "2j3MGgjTZnf5woD1dV9XScaSy5SxPeKh5eTTzcpZ142z" };

        connect(&m_webSocket, &QWebSocket::connected, this,
            &SolanaClient::onConnected);
        connect(&m_webSocket, &QWebSocket::disconnected, this,
            &SolanaClient::onDisconnected);
        connect(&m_webSocket, &QWebSocket::textMessageReceived, this,
            &SolanaClient::onTextMessageReceived);

        connect(&m_heartbeatTimer, &QTimer::timeout, this,
            &SolanaClient::sendHeartbeat);

        m_webSocket.open(url);
    }

private Q_SLOTS:

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

        m_webSocket.sendTextMessage(subscriptionMessage);

        m_heartbeatTimer.start(60000);
    }

    void onTextMessageReceived(const QString& message)
    {
        try {
            auto data = json::parse(message.toStdString());
            if (data.contains("params")) {
                auto params = data["params"];
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
                                /*bool createInstructionFound = std::any_of(
                                    logs.begin(), logs.end(),
                                    [](const std::string& log) {
                                        return log.find("Program log: "
                                                        "Instruction: Create")
                                            != std::string::npos;
                                    });*/

                                bool swapInstructionFound
                                    = std::any_of(logs.begin(), logs.end(),
                                        [](const std::string& log) {
                                            return log.find("Program log: "
                                                            "Instruction: Swap")
                                                != std::string::npos;
                                        });

                                if (swapInstructionFound
                                    && foundDexName == "Raydium CLMM") {
                                    for (const auto& logEntry : logs) {
                                        std::string log
                                            = logEntry
                                                  .template get<std::string>();
                                        if (log.find("Program data:")
                                            != std::string::npos) {
                                            try {
                                                std::string encoded
                                                    = log.substr(
                                                        log.find(": ") + 2);
                                                std::string decoded
                                                    = base64Decode(encoded);

                                                try {
                                                    auto parsed
                                                        = parse_swap_event(
                                                            decoded);
                                                    if (!parsed.empty()) {
                                                        std::cout
                                                            << std::string(
                                                                   50, '-')
                                                            << std::endl;
                                                        for (const auto& [key,
                                                                 value] :
                                                            parsed.items()) {
                                                            std::cout
                                                                << key << ": "
                                                                << value
                                                                << std::endl;
                                                        }
                                                        std::cout
                                                            << std::string(
                                                                   50, '-')
                                                            << std::endl;
                                                    }
                                                } catch (
                                                    const std::exception& e) {
                                                    std::cerr
                                                        << foundDexName
                                                        << " - Error parse "
                                                           "SwapEvent: "
                                                        << e.what()
                                                        << std::endl;
                                                }
                                            } catch (const std::exception& e) {
                                                std::cerr
                                                    << "Failed to decode: "
                                                    << log << std::endl;
                                                std::cerr
                                                    << "Error: " << e.what()
                                                    << std::endl;
                                            }
                                        }
                                    }
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

    void onDisconnected()
    {
        qDebug() << "WebSocket disconnected";
        m_heartbeatTimer.stop();
        QTimer::singleShot(5000, this, &SolanaClient::reconnect);
    }

    void reconnect()
    {
        qDebug() << "Reconnecting...";
        m_webSocket.open(m_url);
    }

    void sendHeartbeat()
    {
        qDebug() << "Sending heartbeat";
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
