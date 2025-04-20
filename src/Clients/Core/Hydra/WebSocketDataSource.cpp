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

#include "WebSocketDataSource.h"

#include <algorithm>

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Daitengu::Clients::Hydra {

WebSocketDataSource::WebSocketDataSource(const QString& name,
    const QString& wsUrl, const Config& config, QObject* parent)
    : DataSource(parent)
    , name_(name)
    , wsUrl_(wsUrl)
    , config_(config)
{
    webSocket_ = std::make_unique<QWebSocket>();
    pingTimer_ = new QTimer(this);
    batchTimer_ = new QTimer(this);
    connectTimeoutTimer_ = new QTimer(this);

    symbolList_ = QStringList { "BTCUSDT", "ETHUSDT" };

    connect(webSocket_.get(), &QWebSocket::connected, this, [this]() {
        spdlog::info("WebSocketDataSource {}: Connected to {}",
            name_.toStdString(), wsUrl_.toStdString());
        reconnectAttempts_ = 0;
        connectTimeoutTimer_->stop();
        subscribeToSymbols();
        pingTimer_->start(30000);
        batchTimer_->start(500);
    });

    connect(webSocket_.get(), &QWebSocket::disconnected, this, [this]() {
        spdlog::warn(
            "WebSocketDataSource {}: Disconnected", name_.toStdString());
        pingTimer_->stop();
        batchTimer_->stop();
        if (running_) {
            scheduleReconnect(1000);
        }
    });

    connect(webSocket_.get(), &QWebSocket::textMessageReceived, this,
        &WebSocketDataSource::processMessage);

    connect(connectTimeoutTimer_, &QTimer::timeout, this, [this]() {
        spdlog::warn("WebSocketDataSource {}: Connection timed out",
            name_.toStdString());
        webSocket_->close();
        if (running_) {
            scheduleReconnect(1000);
        }
    });

    connect(pingTimer_, &QTimer::timeout, this, &WebSocketDataSource::sendPing);

    connect(
        batchTimer_, &QTimer::timeout, this, &WebSocketDataSource::batchUpdate);
}

WebSocketDataSource::~WebSocketDataSource()
{
    stop();
}

void WebSocketDataSource::start()
{
    if (running_)
        return;
    running_ = true;
    spdlog::info("WebSocketDataSource {}: Starting", name_.toStdString());
    connectWebSocket();
}

void WebSocketDataSource::stop()
{
    if (!running_)
        return;
    running_ = false;
    webSocket_->close();
    pingTimer_->stop();
    batchTimer_->stop();
    connectTimeoutTimer_->stop();
    reconnectAttempts_ = 0;
    spdlog::info("WebSocketDataSource {}: Stopped", name_.toStdString());
}

QVariantMap WebSocketDataSource::getData() const
{
    QMutexLocker locker(&dataMutex_);
    return prices_;
}

void WebSocketDataSource::setSymbolList(const QStringList& symbols)
{
    QMutexLocker locker(&dataMutex_);
    symbolList_ = symbols;
    prices_.clear();
    pendingPrices_.clear();
    spdlog::info("WebSocketDataSource {}: Symbol list updated: {}",
        name_.toStdString(), symbols.join(",").toStdString());
    if (running_ && webSocket_->state() == QAbstractSocket::ConnectedState) {
        subscribeToSymbols();
    }
}

void WebSocketDataSource::setBatchInterval(int ms)
{
    if (ms > 0) {
        batchTimer_->setInterval(ms);
        spdlog::info("WebSocketDataSource {}: Batch interval set to {}ms",
            name_.toStdString(), ms);
    }
}

QCoro::Task<void> WebSocketDataSource::connectWebSocket()
{
    if (!running_ || reconnectAttempts_ >= MAX_RECONNECT_ATTEMPTS) {
        if (reconnectAttempts_ >= MAX_RECONNECT_ATTEMPTS) {
            spdlog::error(
                "WebSocketDataSource {}: Max reconnect attempts reached",
                name_.toStdString());
            Q_EMIT errorOccurred(name_, "Max reconnect attempts reached");
        }
        co_return;
    }

    spdlog::info("WebSocketDataSource {}: Connecting to {} (attempt {}/{})",
        name_.toStdString(), wsUrl_.toStdString(), reconnectAttempts_ + 1,
        MAX_RECONNECT_ATTEMPTS);
    connectTimeoutTimer_->start(CONNECT_TIMEOUT_MS);
    co_await qCoro(webSocket_.get())
        .open(QUrl(wsUrl_), std::chrono::milliseconds(5000));
}

void WebSocketDataSource::processMessage(const QString& message)
{
    try {
        std::string jsonStr = message.toStdString();
        spdlog::debug("WebSocketDataSource {}: Received message: {}",
            name_.toStdString(), jsonStr);

        json j = json::parse(jsonStr);
        if (j.is_null() || !j.contains("e")
            || j["e"] != config_.messageType.toStdString()) {
            spdlog::debug("WebSocketDataSource {}: Ignored non-{} message",
                name_.toStdString(), config_.messageType.toStdString());
            return;
        }

        QString symbol = QString::fromStdString(
            j[config_.symbolField.toStdString()].get<std::string>());
        double price
            = std::stod(j[config_.priceField.toStdString()].get<std::string>());

        {
            QMutexLocker locker(&dataMutex_);
            pendingPrices_[symbol] = price;
            spdlog::debug("WebSocketDataSource {}: Queued price for {}: {}",
                name_.toStdString(), symbol.toStdString(), price);
        }
    } catch (const json::exception& e) {
        spdlog::warn("WebSocketDataSource {}: JSON parsing error: {}",
            name_.toStdString(), e.what());
        Q_EMIT errorOccurred(
            name_, QString("JSON parsing error: %1").arg(e.what()));
    } catch (const std::exception& e) {
        spdlog::error("WebSocketDataSource {}: Unexpected error: {}",
            name_.toStdString(), e.what());
        Q_EMIT errorOccurred(
            name_, QString("Unexpected error: %1").arg(e.what()));
    }
}

void WebSocketDataSource::subscribeToSymbols()
{
    if (symbolList_.isEmpty()) {
        spdlog::warn("WebSocketDataSource {}: Symbol list is empty",
            name_.toStdString());
        Q_EMIT errorOccurred(name_, "Empty symbol list");
        return;
    }

    json subscribeMsg;
    subscribeMsg["method"] = "SUBSCRIBE";
    for (const auto& symbol : symbolList_) {
        QString formattedSymbol = config_.formatSymbol(symbol);
        subscribeMsg["params"].push_back(formattedSymbol.toStdString());
    }
    subscribeMsg["id"] = 1;

    QString message = QString::fromStdString(subscribeMsg.dump());
    webSocket_->sendTextMessage(message);
    spdlog::debug("WebSocketDataSource {}: Sent subscription: {}",
        name_.toStdString(), message.toStdString());
}

void WebSocketDataSource::sendPing()
{
    if (webSocket_->state() == QAbstractSocket::ConnectedState) {
        webSocket_->ping();
        spdlog::debug("WebSocketDataSource {}: Sent ping", name_.toStdString());
    } else {
        spdlog::warn(
            "WebSocketDataSource {}: Ping failed, WebSocket not connected",
            name_.toStdString());
        if (running_) {
            scheduleReconnect(1000);
        }
    }
}

void WebSocketDataSource::batchUpdate()
{
    QMutexLocker locker(&dataMutex_);
    if (pendingPrices_.isEmpty()) {
        return;
    }

    if (pendingPrices_ != prices_) {
        prices_ = pendingPrices_;
        spdlog::info(
            "WebSocketDataSource {}: Batch price update", name_.toStdString());
        Q_EMIT dataUpdated(name_, prices_);
    } else {
        spdlog::debug("WebSocketDataSource {}: No batch price changes",
            name_.toStdString());
    }
    pendingPrices_.clear();
}

void WebSocketDataSource::scheduleReconnect(int delayMs)
{
    reconnectAttempts_++;
    int cappedDelay
        = std::min(delayMs << reconnectAttempts_, MAX_RECONNECT_DELAY_MS);
    spdlog::info(
        "WebSocketDataSource {}: Scheduling reconnect in {}ms (attempt {}/{})",
        name_.toStdString(), cappedDelay, reconnectAttempts_,
        MAX_RECONNECT_ATTEMPTS);
    QTimer::singleShot(cappedDelay, this, [this]() { connectWebSocket(); });
}
}
