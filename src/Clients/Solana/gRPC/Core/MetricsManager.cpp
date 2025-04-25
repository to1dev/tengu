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

#include "MetricsManager.hpp"

#include "../Utils/Logger.hpp"

namespace solana {

MetricsManager::MetricsManager(uint16_t port, QObject* parent)
    : QObject(parent)
{
    try {
        registry_ = std::make_shared<prometheus::Registry>();

        // Initialize transaction counter family
        transaction_counter_family_
            = &prometheus::BuildCounter()
                   .Name("solana_transactions_total")
                   .Help("Total transactions processed by source")
                   .Register(*registry_);

        // Initialize filter hits counter family
        filter_hits_family_ = &prometheus::BuildCounter()
                                   .Name("solana_filter_hits_total")
                                   .Help("Total filter hits by filter name")
                                   .Register(*registry_);

        // Initialize connection status gauge family
        connection_status_family_
            = &prometheus::BuildGauge()
                   .Name("solana_connection_status")
                   .Help("Connection status of data sources (1=connected, "
                         "0=disconnected)")
                   .Register(*registry_);

        // Start Prometheus exposer
        prometheus::Exposer exposer { "0.hover" };
        exposer.RegisterCollectable(registry_);
        Logger::getLogger()->info(
            "MetricsManager initialized on port {}", port);

    } catch (const std::exception& e) {
        Logger::getLogger()->error(
            "MetricsManager initialization failed: {}", e.what());
    }
}

MetricsManager::~MetricsManager() = default;

void MetricsManager::incrementTransactionCount(
    const std::string& sourceId, uint64_t count)
{
    try {
        std::lock_guard lock(mutex_);
        auto& counter
            = transaction_counter_family_->Add({ { "source_id", sourceId } });
        counter.Increment(count);
        Q_EMIT metricsUpdated(
            QString::fromStdString("solana_transactions_total"), count);
    } catch (const std::exception& e) {
        Logger::getLogger()->error(
            "Failed to increment transaction count: {}", e.what());
    }
}

void MetricsManager::incrementFilterHits(
    const std::string& filterName, uint64_t hits)
{
    try {
        std::lock_guard lock(mutex_);
        auto& counter
            = filter_hits_family_->Add({ { "filter_name", filterName } });
        counter.Increment(hits);
        Q_EMIT metricsUpdated(
            QString::fromStdString("solana_filter_hits_total"), hits);
    } catch (const std::exception& e) {
        Logger::getLogger()->error(
            "Failed to increment filter hits: {}", e.what());
    }
}

void MetricsManager::setConnectionStatus(
    const std::string& sourceId, bool connected)
{
    try {
        std::lock_guard lock(mutex_);
        auto& gauge
            = connection_status_family_->Add({ { "source_id", sourceId } });
        gauge.Set(connected ? 1.0 : 0.0);
        Q_EMIT metricsUpdated(
            QString::fromStdString("solana_connection_status"),
            connected ? 1.0 : 0.0);
    } catch (const std::exception& e) {
        Logger::getLogger()->error(
            "Failed to set connection status: {}", e.what());
    }
}
}
