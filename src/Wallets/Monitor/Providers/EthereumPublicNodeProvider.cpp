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

#include "EthereumPublicNodeProvider.h"

#include <QNetworkReply>

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Daitengu::Wallets {

EthereumPublicNodeProvider::EthereumPublicNodeProvider()
{
}

QCoro::Task<std::optional<Monitor::BalanceResult>>
EthereumPublicNodeProvider::fetchBalance(QNetworkAccessManager* networkManager,
    const QString& address, ChainType chain)
{
    Monitor::BalanceResult result {
        .address = address,
        .chain = ChainType::ETHEREUM,
        .dataSource = apiEndpoint_,
    };

    try {
        QNetworkRequest request { QUrl(apiEndpoint_) };
        request.setHeader(
            QNetworkRequest::ContentTypeHeader, "application/json");

        json rpcRequest = {
            { "jsonrpc", "2.0" },
            { "id", 1 },
            { "method", "eth_getBalance" },
            { "params", { address.toStdString(), "latest" } },
        };

        auto reply = std::unique_ptr<QNetworkReply>(networkManager->post(
            request, QByteArray::fromStdString(rpcRequest.dump())));

        bool finished = co_await qCoro(reply.get())
                            .waitForFinished(std::chrono::milliseconds(5000));

        if (!finished) {
            result.errorMessage = "Network request timed out";
            spdlog::error(
                "Network timeout for address {}: request took too long",
                address.toStdString());
            co_return std::nullopt;
        }

        if (reply->error() != QNetworkReply::NoError) {
            result.errorMessage = reply->errorString();
            if (reply->error() == QNetworkReply::HostNotFoundError) {
                spdlog::critical("Host not found for address {}: {}",
                    address.toStdString(), result.errorMessage.toStdString());
            } else {
                spdlog::warn("Network error for address {}: {} (Code: {})",
                    address.toStdString(), result.errorMessage.toStdString(),
                    static_cast<int>(reply->error()));
            }
            co_return std::nullopt;
        }

        json j = json::parse(reply->readAll().toStdString());
        try {
            std::string balanceHex = j["result"].get<std::string>();
            if (balanceHex.substr(0, 2) == "0x") {
                balanceHex = balanceHex.substr(2);
            }
            result.balance
                = std::stoull(balanceHex, nullptr, 16) / 1e18; // Wei to ETH
            result.success = true;
            spdlog::info("ETH balance for {}: {}", address.toStdString(),
                result.balance);
        } catch (const json::exception& e) {
            result.errorMessage = QString::fromStdString(e.what());
            spdlog::error("JSON parsing error for ETH: {}", e.what());
        }
    } catch (const std::exception& e) {
        result.errorMessage = QString::fromStdString(e.what());
        spdlog::error("Exception in Ethereum PublicNode fetch for {}: {}",
            address.toStdString(), e.what());
        co_return std::nullopt;
    }

    co_return result.success ? std::make_optional(result) : std::nullopt;
}

QString EthereumPublicNodeProvider::getApiEndpoint() const
{
    return apiEndpoint_;
}
}
