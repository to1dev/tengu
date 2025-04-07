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

#include "Monitor.h"

#include <QNetworkReply>
#include <QTimer>

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace std::string_literals;

namespace Daitengu::Wallets {

class MonitorPrivate {
public:
    explicit MonitorPrivate(Monitor* q)
        : q_ptr(q)
    {
        networkManager_ = std::make_unique<QNetworkAccessManager>();
        refreshTimer_ = std::make_unique<QTimer>();
        refreshTimer_->setInterval(DEFAUlT_INTERVAL);

        QObject::connect(refreshTimer_.get(), &QTimer::timeout, q, [this]() {
            if (currentChain_ != ChainType::UNKNOWN) {
                q_ptr->refresh();
            }
        });

        spdlog::info("Monitor initialized");
    }

    Monitor* q_ptr;
    std::unique_ptr<QNetworkAccessManager> networkManager_;
    std::unique_ptr<QTimer> refreshTimer_;
    ChainType currentChain_ { ChainType::UNKNOWN };
    QString currentAddress_;
};

Monitor::Monitor(QObject* parent)
    : QObject(parent)
    , d_ptr(std::make_unique<MonitorPrivate>(this))
{
    Q_D(Monitor);
    d->refreshTimer_->start();
}

Monitor::~Monitor()
{
}

void Monitor::setAddress(ChainType chain, const QString& address)
{
    Q_D(Monitor);
    if (d->currentChain_ != chain || d->currentAddress_ != address) {
        d->currentChain_ = chain;
        d->currentAddress_ = address;
        spdlog::info("Monitoring address changed to {} on chain {}",
            address.toStdString(), static_cast<int>(chain));
        Q_EMIT addressChanged(chain, address);
        if (chain != ChainType::UNKNOWN) {
            d->refreshTimer_->start();
            fetchBalance(d->currentChain_, d->currentAddress_);
        }
    }
}

std::pair<ChainType, QString> Monitor::getAddress() const
{
    Q_D(const Monitor);
    return { d->currentChain_, d->currentAddress_ };
}

void Monitor::setRefreshInterval(int interval)
{
    Q_D(Monitor);
    d->refreshTimer_->setInterval(interval);
    spdlog::info("Refresh interval set to {} ms", interval);
}

void Monitor::refresh()
{
    Q_D(Monitor);
    if (d->currentChain_ != ChainType::UNKNOWN) {
        spdlog::debug("Refreshing balance for {} on chain {}",
            d->currentAddress_.toStdString(),
            static_cast<int>(d->currentChain_));
        fetchBalance(d->currentChain_, d->currentAddress_);
    }
}

QCoro::Task<std::optional<Monitor::BalanceResult>> Monitor::fetchBalance(
    ChainType chain, const QString& address)
{
    Q_D(Monitor);

    BalanceResult result {
        .address = address,
        .chain = chain,
    };

    try {
        switch (chain) {
        case ChainType::BITCOIN: {
            QNetworkRequest request { QUrl(
                QString("%1%2").arg(MEMPOOL_API).arg(address)) };
            auto reply = std::unique_ptr<QNetworkReply>(
                d->networkManager_->get(request));
            co_await qCoro(reply.get())
                .waitForFinished(std::chrono::milliseconds(5000));
            if (!reply->error()) {
                auto data = reply->readAll();
                try {
                    json j = json::parse(data.toStdString());
                    if (j.is_object() && j.contains("chain_stats")
                        && j["chain_stats"].contains("funded_txo_sum")) {
                        uint64_t confirmed
                            = j["chain_stats"]["funded_txo_sum"].get<uint64_t>()
                            - j["chain_stats"]["spent_txo_sum"].get<uint64_t>();

                        uint64_t unconfirmed
                            = j["mempool_stats"]["funded_txo_sum"]
                                  .get<uint64_t>()
                            - j["mempool_stats"]["spent_txo_sum"]
                                  .get<uint64_t>();
                        double btcAmount
                            = static_cast<double>(confirmed + unconfirmed)
                            / 1e8;
                        /*auto chain_stats = j.at("chain_stats");
                        auto funded
                            = chain_stats.at("funded_txo_sum").get<double>();
                        auto spent
                            = chain_stats.at("spent_txo_sum").get<double>();
                        result.balance = (funded - spent) / 1e8;*/
                        result.balance = btcAmount;
                        result.success = true;

                        spdlog::info("BTC balance for {}: {}",
                            address.toStdString(), result.balance);
                    }
                } catch (const json::exception& e) {
                    result.errorMessage = QString::fromStdString(e.what());
                    spdlog::error("JSON parsing error for BTC: {}", e.what());
                }
            }

            break;
        }

        case ChainType::ETHEREUM: {
            d->refreshTimer_->stop();
            result.errorMessage = "Ethereum monitoring not implemented yet";
            spdlog::warn("ETH not implemented for {}", address.toStdString());
            co_await QCoro::sleepFor(std::chrono::seconds(1));
            break;
        }

        case ChainType::SOLANA: {
            d->refreshTimer_->stop();
            result.errorMessage = "Solana monitoring not implemented yet";
            spdlog::warn("SOL not implemented for {}", address.toStdString());
            co_await QCoro::sleepFor(std::chrono::seconds(1));
            break;
        }

        case ChainType::SUI: {
            d->refreshTimer_->stop();
            result.errorMessage = "Sui monitoring not implemented yet";
            spdlog::warn("SUI not implemented for {}", address.toStdString());
            co_await QCoro::sleepFor(std::chrono::seconds(1));
            break;
        }

        case ChainType::UNKNOWN:
        default:
            d->refreshTimer_->stop();
            result.errorMessage = "No chain specified";
            spdlog::warn("Attempted to fetch balance with unknown chain");
            break;
        }
    } catch (const std::exception& e) {
        result.errorMessage = e.what();
        spdlog::error("Exception during balance fetch: {}", e.what());
    }

    Q_EMIT balanceUpdated(result);

    co_return result.success ? std::make_optional(result) : std::nullopt;
}
}
