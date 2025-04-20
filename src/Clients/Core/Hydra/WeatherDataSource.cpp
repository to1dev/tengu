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

#include "WeatherDataSource.h"

#include <QNetworkReply>
#include <QTimer>

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Daitengu::Clients::Hydra {

WeatherDataSource::WeatherDataSource(const QString& name, const QString& apiKey,
    const QString& city, int interval, int timeoutMs, QObject* parent)
    : DataSource(parent)
    , name_(name)
    , apiKey_(apiKey)
    , city_(city)
    , interval_(interval)
    , timeoutMs_(timeoutMs)
{
    networkManager_ = std::make_unique<QNetworkAccessManager>();
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, [this]() { updateWeather(); });
}

WeatherDataSource::~WeatherDataSource()
{
    stop();
}

void WeatherDataSource::start()
{
    if (running_)
        return;
    running_ = true;
    spdlog::info("WeatherDataSource {} started with interval {}s",
        name_.toStdString(), interval_);
    QTimer::singleShot(0, this, [this]() { updateWeather(); });
    timer_->start(interval_ * 1000);
}

void WeatherDataSource::stop()
{
    if (!running_)
        return;
    running_ = false;
    timer_->stop();
    spdlog::info("WeatherDataSource {} stopped", name_.toStdString());
}

QVariantMap WeatherDataSource::getData() const
{
    QMutexLocker locker(&dataMutex_);
    return weatherData_;
}

QCoro::Task<void> WeatherDataSource::updateWeather()
{
    try {
        if (!running_)
            co_return;
        if (apiKey_.isEmpty() || city_.isEmpty()) {
            spdlog::warn("WeatherDataSource {}: API key or city is empty",
                name_.toStdString());
            Q_EMIT errorOccurred(name_, "API key or city is empty");
            co_return;
        }

        QString url = QString("https://api.openweathermap.org/data/2.5/"
                              "weather?q=%1&appid=%2&units=metric")
                          .arg(city_, apiKey_);
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
                "WeatherDataSource {}: Network request failed (retry {}/3): {}",
                name_.toStdString(), retry + 1,
                reply->errorString().toStdString());
            co_await QCoro::sleepFor(
                std::chrono::milliseconds(1000 * (retry + 1)));
        }

        if (reply->error()) {
            QString error = QString("Network request failed after retries: %1")
                                .arg(reply->errorString());
            spdlog::error("WeatherDataSource {}: {}", name_.toStdString(),
                error.toStdString());
            Q_EMIT errorOccurred(name_, error);
            co_return;
        }

        QByteArray data = reply->readAll();
        std::string jsonStr = data.toStdString();
        spdlog::debug("WeatherDataSource {}: Received data: {}",
            name_.toStdString(), jsonStr);

        json j = json::parse(jsonStr);
        if (j.is_null() || !j.contains("main") || !j.contains("weather")) {
            spdlog::warn("WeatherDataSource {}: Invalid JSON response",
                name_.toStdString());
            Q_EMIT errorOccurred(name_, "Invalid JSON response");
            co_return;
        }

        QVariantMap newWeather;
        newWeather["temperature"] = j["main"]["temp"].get<double>();
        newWeather["humidity"] = j["main"]["humidity"].get<int>();
        newWeather["description"] = QString::fromStdString(
            j["weather"][0]["description"].get<std::string>());

        {
            QMutexLocker locker(&dataMutex_);
            if (newWeather != weatherData_) {
                weatherData_ = newWeather;
                spdlog::info("WeatherDataSource {}: Weather updated for {}",
                    name_.toStdString(), city_.toStdString());
                Q_EMIT dataUpdated(name_, weatherData_);
            } else {
                spdlog::debug(
                    "WeatherDataSource {}: No weather change detected",
                    name_.toStdString());
            }
        }
    } catch (const json::exception& e) {
        spdlog::warn("WeatherDataSource {}: JSON parsing error: {}",
            name_.toStdString(), e.what());
        Q_EMIT errorOccurred(
            name_, QString("JSON parsing error: %1").arg(e.what()));
    } catch (const std::exception& e) {
        spdlog::error("WeatherDataSource {}: Unexpected error: {}",
            name_.toStdString(), e.what());
        Q_EMIT errorOccurred(
            name_, QString("Unexpected error: %1").arg(e.what()));
    }
}
}
