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

#include "BitcoinMempoolProvider.h"

#include <QNetworkReply>

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Daitengu::Wallets {

BitcoinMempoolProvider::BitcoinMempoolProvider()
{
}

QCoro::Task<std::optional<Monitor::BalanceResult>>
BitcoinMempoolProvider::fetchBalance(QNetworkAccessManager* networkManager,
    const QString& address, ChainType chain)
{
    Monitor::BalanceResult result {
        .address = address,
        .chain = chain,
        .dataSource = apiEndpoint_,
    };

    try {
        QNetworkRequest request { QUrl(apiEndpoint_ + address) };
        auto reply
            = std::unique_ptr<QNetworkReply>(networkManager->get(request));
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
            if (j.is_object() && j.contains("chain_stats")
                && j["chain_stats"].contains("funded_txo_sum")) {
                uint64_t confirmed
                    = j["chain_stats"]["funded_txo_sum"].get<uint64_t>()
                    - j["chain_stats"]["spent_txo_sum"].get<uint64_t>();
                uint64_t unconfirmed
                    = j["mempool_stats"]["funded_txo_sum"].get<uint64_t>()
                    - j["mempool_stats"]["spent_txo_sum"].get<uint64_t>();
                result.balance
                    = static_cast<double>(confirmed + unconfirmed) / 1e8;
                result.success = true;
                spdlog::info("BTC balance for {}: {}", address.toStdString(),
                    result.balance);
            }
        } catch (const json::exception& e) {
            result.errorMessage = QString::fromStdString(e.what());
            spdlog::error("JSON parsing error for BTC: {}", e.what());
        }
    } catch (const std::exception& e) {
        result.errorMessage = e.what();
        spdlog::error("Exception in Bitcoin Mempool fetch for {}: {}",
            address.toStdString(), e.what());
    }

    co_return result.success ? std::make_optional(result) : std::nullopt;
}

QString BitcoinMempoolProvider::getApiEndpoint() const
{
    return apiEndpoint_;
}
}
