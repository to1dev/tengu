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

namespace Daitengu::Wallets {

class MonitorPrivate {
public:
    explicit MonitorPrivate(Monitor* q)
        : q_ptr(q)
    {
        networkManager_ = std::make_unique<QNetworkAccessManager>();
        refreshTimer_ = std::make_unique<QTimer>();
        refreshTimer_->setInterval(60000);

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
    spdlog::info("Monitor destroyed");
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
            fetchBalance(chain, address);
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
    return QCoro::Task<std::optional<BalanceResult>>();
}
}
