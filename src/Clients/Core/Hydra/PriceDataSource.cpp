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

#include "PriceDataSource.h"

#include <QNetworkReply>
#include <QTimer>

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Daitengu::Clients {

inline constexpr char defaultApi[] = "https://min-api.cryptocompare.com/data/"
                                     "pricemulti?fsyms=%1&tsyms=USD,EUR";

PriceDataSource::PriceDataSource(
    const QString& name, int interval, int timeoutMs, QObject* parent)
    : DataSource(parent)
    , name_(name)
    , interval_(interval)
    , timeoutMs_(timeoutMs)
    , apiUrl_(defaultApi)
{
    networkManager_ = std::make_unique<QNetworkAccessManager>();
    tickerList_ = QStringList { "BTC", "ETH", "SOL", "SUI" };
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, [this]() { updatePrices(); });
}

PriceDataSource::~PriceDataSource()
{
    stop();
}

void PriceDataSource::start()
{
    if (running_)
        return;
    running_ = true;
    spdlog::info("PriceDataSource {} started with interval {}s",
        name_.toStdString(), interval_);
    QTimer::singleShot(0, this, [this]() { updatePrices(); });
    timer_->start(interval_ * 1000);
}

void PriceDataSource::stop()
{
    if (!running_)
        return;
    running_ = false;
    timer_->stop();
    spdlog::info("PriceDataSource {} stopped", name_.toStdString());
}

QVariantMap PriceDataSource::getData() const
{
    QMutexLocker locker(&dataMutex_);
    return prices_;
}

void PriceDataSource::setTickerList(const QStringList& tickers)
{
    QMutexLocker locker(&dataMutex_);
    tickerList_ = tickers;
    prices_.clear();
    spdlog::info("PriceDataSource {} ticker list updated: {}",
        name_.toStdString(), tickers.join(",").toStdString());
}

void PriceDataSource::setApiUrl(const QString& apiUrl)
{
    QMutexLocker locker(&dataMutex_);
    apiUrl_ = apiUrl.isEmpty() ? defaultApi : apiUrl;
    spdlog::info("PriceDataSource {} API URL set to: {}", name_.toStdString(),
        apiUrl_.toStdString());
}

QCoro::Task<void> PriceDataSource::updatePrices()
{
    try {
        if (!running_)
            co_return;
        if (tickerList_.isEmpty()) {
            spdlog::warn("PriceDataSource {}: Ticker list is empty",
                name_.toStdString());
            Q_EMIT errorOccurred(name_, "Empty ticker list");
            co_return;
        }

        QString tickers = tickerList_.join(",");
        QString url = apiUrl_.arg(tickers);
        QNetworkRequest request { QUrl(url) };
        std::unique_ptr<QNetworkReply> reply;

        for (int retry = 0; retry < 3; ++retry) {
            reply
                = std::unique_ptr<QNetworkReply>(networkManager_->get(request));
            co_await qCoro(reply.get())
                .waitForFinished(std::chrono::milliseconds(timeoutMs_));
            if (!reply->error())
                break;
            spdlog::warn(
                "PriceDataSource {}: Network request failed (retry {}/3): {}",
                name_.toStdString(), retry + 1,
                reply->errorString().toStdString());
            co_await QCoro::sleepFor(
                std::chrono::milliseconds(1000 * (retry + 1)));
        }

        if (reply->error()) {
            QString error = QString("Network request failed after retries: %1")
                                .arg(reply->errorString());
            spdlog::error("PriceDataSource {}: {}", name_.toStdString(),
                error.toStdString());
            Q_EMIT errorOccurred(name_, error);
            co_return;
        }

        QByteArray data = reply->readAll();
        std::string jsonStr = data.toStdString();
        spdlog::debug("PriceDataSource {}: Received data: {}",
            name_.toStdString(), jsonStr);

        json j = json::parse(jsonStr);
        if (j.is_null() || !j.is_object()) {
            spdlog::warn("PriceDataSource {}: Invalid JSON response",
                name_.toStdString());
            Q_EMIT errorOccurred(name_, "Invalid JSON response");
            co_return;
        }

        QVariantMap newPrices;
        for (const auto& ticker : tickerList_) {
            std::string tickerStr = ticker.toStdString();
            if (j.contains(tickerStr)) {
                if (j[tickerStr].contains("USD")) {
                    newPrices[QString("%1_USD").arg(ticker)]
                        = j[tickerStr]["USD"].get<double>();
                }
                if (j[tickerStr].contains("EUR")) {
                    newPrices[QString("%1_EUR").arg(ticker)]
                        = j[tickerStr]["EUR"].get<double>();
                }
            }
        }

        {
            QMutexLocker locker(&dataMutex_);
            if (newPrices != prices_) {
                prices_ = newPrices;
                spdlog::info(
                    "PriceDataSource {}: Prices updated for tickers: {}",
                    name_.toStdString(), tickers.toStdString());
                Q_EMIT dataUpdated(name_, prices_);
            } else {
                spdlog::debug("PriceDataSource {}: No price changes detected",
                    name_.toStdString());
            }
        }
    } catch (const json::exception& e) {
        spdlog::warn("PriceDataSource {}: JSON parsing error: {}",
            name_.toStdString(), e.what());
        Q_EMIT errorOccurred(
            name_, QString("JSON parsing error: %1").arg(e.what()));
    } catch (const std::exception& e) {
        spdlog::error("PriceDataSource {}: Unexpected error: {}",
            name_.toStdString(), e.what());
        Q_EMIT errorOccurred(
            name_, QString("Unexpected error: %1").arg(e.what()));
    }
}
}
