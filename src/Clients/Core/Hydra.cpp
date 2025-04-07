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

#include "Hydra.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Daitengu::Clients {

Hydra::Hydra(QObject* parent, int interval)
    : QObject(parent)
    , interval_(interval)
{
    networkManager_ = std::make_unique<QNetworkAccessManager>();
    tickerList_ = QStringList { "btc", "eth" };
}

Hydra::~Hydra()
{
    stop();
}

void Hydra::start()
{
    running_ = true;
    updatePrices();
}

void Hydra::stop()
{
    running_ = false;
}

void Hydra::setTickerList(const QStringList& tickers)
{
    QMutexLocker locker(&priceMutex_);
    tickerList_ = tickers;
    prices_.clear();
}

double Hydra::getPrice(const QString& ticker) const
{
    QMutexLocker locker(&priceMutex_);
    return prices_.value(ticker, 0.0);
}

QMap<QString, double> Hydra::getPrices() const
{
    QMutexLocker locker(&priceMutex_);
    return prices_;
}

QCoro::Task<void> Hydra::updatePrices()
{
    while (running_) {
        for (const auto& ticker : tickerList_) {
            if (ticker == "btc") {
                QNetworkRequest request { QUrl(defaultApi) };
                auto reply = std::unique_ptr<QNetworkReply>(
                    networkManager_->get(request));
                // co_await reply.get();
                co_await qCoro(reply.get())
                    .waitForFinished(std::chrono::milliseconds(5000));
                if (!reply->error()) {
                    QByteArray data = reply->readAll();
                    std::string jsonStr = data.toStdString();

                    try {
                        json j = json::parse(jsonStr);
                        if (!j.is_null() && j.is_object()) {
                            double price = j["USD"].get<double>();
                            {
                                QMutexLocker locker(&priceMutex_);
                                prices_[ticker] = price;
                            }
                            Q_EMIT priceUpdated(ticker, price);
                        }
                    } catch (const json::exception& e) {
                        spdlog::warn("JSON parsing error: {}", e.what());
                    }
                }
            }
        }

        co_await QCoro::sleepFor(std::chrono::seconds(interval_));
    }
}
}
