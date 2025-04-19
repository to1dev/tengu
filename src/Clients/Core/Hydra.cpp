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

inline constexpr char defaultApi[] = "https://min-api.cryptocompare.com/data/"
                                     "pricemulti?fsyms=%1&tsyms=USD,EUR";

Hydra::Hydra(QObject* parent, int interval, int timeoutMs)
    : QObject(parent)
    , interval_(interval)
    , timeoutMs_(timeoutMs)
    , apiUrl_(defaultApi)
{
    networkManager_ = std::make_unique<QNetworkAccessManager>();
    tickerList_ = QStringList { "BTC", "ETH", "SOL", "SUI" };
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, [this]() { updatePrices(); });
}

Hydra::~Hydra()
{
    stop();
}

void Hydra::start()
{
    if (running_)
        return;
    running_ = true;
    spdlog::info("Hydra started with interval {}s", interval_);

    QTimer::singleShot(0, this, [this]() { updatePrices(); });

    timer_->start(interval_ * 1000);
}

void Hydra::stop()
{
    if (!running_)
        return;
    running_ = false;
    timer_->stop();
    spdlog::info("Hydra stopped");
}

void Hydra::setTickerList(const QStringList& tickers)
{
    QMutexLocker locker(&priceMutex_);
    tickerList_ = tickers;
    prices_.clear();
    spdlog::info("Ticker list updated: {}", tickers.join(",").toStdString());
}

void Hydra::setApiUrl(const QString& apiUrl)
{
    QMutexLocker locker(&priceMutex_);
    apiUrl_ = apiUrl.isEmpty() ? defaultApi : apiUrl;
    spdlog::info("API URL set to: {}", apiUrl_.toStdString());
}

double Hydra::getPrice(const QString& ticker, const QString& currency) const
{
    QMutexLocker locker(&priceMutex_);
    QString key = QString("%1_%2").arg(ticker, currency);
    return prices_.value(key, 0.0);
}

QMap<QString, double> Hydra::getPrices() const
{
    QMutexLocker locker(&priceMutex_);
    return prices_;
}

QCoro::Task<void> Hydra::updatePrices()
{
    try {
        if (!running_)
            co_return;
        if (tickerList_.isEmpty()) {
            spdlog::warn("Ticker list is empty, skipping price update");
            Q_EMIT errorOccurred("Empty ticker list");
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
            spdlog::warn("Network request failed (retry {}/3): {}", retry + 1,
                reply->errorString().toStdString());
            co_await QCoro::sleepFor(
                std::chrono::milliseconds(1000 * (retry + 1)));
        }

        if (reply->error()) {
            QString error = QString("Network request failed after retries: %1")
                                .arg(reply->errorString());
            spdlog::error("{}", error.toStdString());
            Q_EMIT errorOccurred(error);
            co_return;
        }

        QByteArray data = reply->readAll();
        std::string jsonStr = data.toStdString();
        spdlog::debug("Received data: {}", jsonStr);

        json j = json::parse(jsonStr);
        if (j.is_null() || !j.is_object()) {
            spdlog::warn("Invalid JSON response");
            Q_EMIT errorOccurred("Invalid JSON response");
            co_return;
        }

        QMap<QString, double> newPrices;
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
            QMutexLocker locker(&priceMutex_);
            if (newPrices != prices_) {
                prices_ = newPrices;
                spdlog::info(
                    "Prices updated for tickers: {}", tickers.toStdString());
                Q_EMIT pricesUpdated(prices_);
            } else {
                spdlog::debug("No price changes detected");
            }
        }
    } catch (const json::exception& e) {
        spdlog::warn("JSON parsing error: {}", e.what());
        Q_EMIT errorOccurred(QString("JSON parsing error: %1").arg(e.what()));
    } catch (const std::exception& e) {
        spdlog::error("Unexpected error in updatePrices: {}", e.what());
        Q_EMIT errorOccurred(QString("Unexpected error: %1").arg(e.what()));
    }
}
}
