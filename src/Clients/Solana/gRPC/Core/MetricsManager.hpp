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

#pragma once

#include <mutex>

#include <QObject>

#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/family.h>
#include <prometheus/gauge.h>
#include <prometheus/registry.h>

namespace solana {

class MetricsManager : public QObject {
    Q_OBJECT

public:
    MetricsManager(uint16_t port = 9090, QObject* parent = nullptr);
    ~MetricsManager();

    void incrementTransactionCount(const std::string& sourceId, uint64_t count);
    void incrementFilterHits(const std::string& filterName, uint64_t hits);
    void setConnectionStatus(const std::string& sourceId, bool connected);

Q_SIGNALS:
    void metricsUpdated(const QString& metricName, double value);

private:
    std::shared_ptr<prometheus::Registry> registry_;
    prometheus::Family<prometheus::Counter>* transaction_counter_family_;
    prometheus::Family<prometheus::Counter>* filter_hits_family_;
    prometheus::Family<prometheus::Gauge>* connection_status_family_;
    mutable std::mutex mutex_;
};
}
