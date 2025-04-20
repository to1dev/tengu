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

#include "SolanaBlockHashDataSource.h"

#include <QNetworkReply>

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Daitengu::Clients::Hydra {

inline constexpr char defaultRpcUrl[] = "https://api.mainnet-beta.solana.com";

SolanaBlockHashDataSource::SolanaBlockHashDataSource(
    const QString& name, const QString& rpcUrl, int timeoutMs, QObject* parent)
    : DataSource(parent)
    , name_(name)
    , rpcUrl_(rpcUrl.isEmpty() ? defaultRpcUrl : rpcUrl)
    , timeoutMs_(timeoutMs)
{
    networkManager_ = std::make_unique<QNetworkAccessManager>();
}

SolanaBlockHashDataSource::~SolanaBlockHashDataSource()
{
    stop();
}

void SolanaBlockHashDataSource::start()
{
    if (running_)
        return;
    running_ = true;
    spdlog::info("SolanaBlockHashDataSource {}: Started", name_.toStdString());
}

void SolanaBlockHashDataSource::stop()
{
    if (!running_)
        return;
    running_ = false;
    spdlog::info("SolanaBlockHashDataSource {}: Stopped", name_.toStdString());
}

QVariantMap SolanaBlockHashDataSource::getData() const
{
    QMutexLocker locker(&dataMutex_);
    return blockHashData_;
}

void SolanaBlockHashDataSource::setRpcUrl(const QString& rpcUrl)
{
    QMutexLocker locker(&dataMutex_);
    rpcUrl_ = rpcUrl.isEmpty() ? defaultRpcUrl : rpcUrl;
    blockHashData_.clear();
    spdlog::info("SolanaBlockHashDataSource {}: RPC URL set to: {}",
        name_.toStdString(), rpcUrl_.toStdString());
}

void SolanaBlockHashDataSource::fetchBlockHash()
{
    if (!running_) {
        spdlog::warn(
            "SolanaBlockHashDataSource {}: Not running, ignoring fetch request",
            name_.toStdString());
        return;
    }
    fetchBlockHashAsync();
}

QCoro::Task<void> SolanaBlockHashDataSource::fetchBlockHashAsync()
{
    try {
        if (!running_)
            co_return;
        spdlog::info(
            "SolanaBlockHashDataSource {}: Fetching block hash from {}",
            name_.toStdString(), rpcUrl_.toStdString());

        json request;
        request["jsonrpc"] = "2.0";
        request["id"] = 1;
        request["method"] = "getLatestBlockhash";
        request["params"] = json::array();

        QNetworkRequest networkRequest { QUrl(rpcUrl_) };
        networkRequest.setHeader(
            QNetworkRequest::ContentTypeHeader, "application/json");
        QByteArray requestData
            = QString::fromStdString(request.dump()).toUtf8();

        std::unique_ptr<QNetworkReply> reply;
        for (int retry = 0; retry < 3; ++retry) {
            reply = std::unique_ptr<QNetworkReply>(
                networkManager_->post(networkRequest, requestData));
            co_await qCoro(reply.get())
                .waitForFinished(std::chrono::milliseconds(timeoutMs_));
            if (!reply->error())
                break;
            spdlog::warn("SolanaBlockHashDataSource {}: Network request failed "
                         "(retry {}/3): {}",
                name_.toStdString(), retry + 1,
                reply->errorString().toStdString());
            co_await QCoro::sleepFor(
                std::chrono::milliseconds(1000 * (retry + 1)));
        }

        if (reply->error()) {
            QString error = QString("Network request failed after retries: %1")
                                .arg(reply->errorString());
            spdlog::error("SolanaBlockHashDataSource {}: {}",
                name_.toStdString(), error.toStdString());
            Q_EMIT errorOccurred(name_, error);
            co_return;
        }

        QByteArray data = reply->readAll();
        std::string jsonStr = data.toStdString();
        spdlog::debug("SolanaBlockHashDataSource {}: Received data: {}",
            name_.toStdString(), jsonStr);

        json response = json::parse(jsonStr);
        if (response.is_null() || !response.contains("result")
            || !response["result"].contains("value")
            || !response["result"]["value"].contains("blockhash")) {
            spdlog::warn("SolanaBlockHashDataSource {}: Invalid JSON response",
                name_.toStdString());
            Q_EMIT errorOccurred(name_, "Invalid JSON response");
            co_return;
        }

        QString blockHash = QString::fromStdString(
            response["result"]["value"]["blockhash"].get<std::string>());
        QVariantMap newData;
        newData["blockHash"] = blockHash;

        {
            QMutexLocker locker(&dataMutex_);
            if (newData != blockHashData_) {
                blockHashData_ = newData;
                spdlog::info(
                    "SolanaBlockHashDataSource {}: Block hash updated: {}",
                    name_.toStdString(), blockHash.toStdString());
                Q_EMIT dataUpdated(name_, blockHashData_);
            } else {
                spdlog::debug(
                    "SolanaBlockHashDataSource {}: No block hash change",
                    name_.toStdString());
            }
        }
    } catch (const json::exception& e) {
        spdlog::warn("SolanaBlockHashDataSource {}: JSON parsing error: {}",
            name_.toStdString(), e.what());
        Q_EMIT errorOccurred(
            name_, QString("JSON parsing error: %1").arg(e.what()));
    } catch (const std::exception& e) {
        spdlog::error("SolanaBlockHashDataSource {}: Unexpected error: {}",
            name_.toStdString(), e.what());
        Q_EMIT errorOccurred(
            name_, QString("Unexpected error: %1").arg(e.what()));
    }
}
}
