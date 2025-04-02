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

#include <iostream>

#include <QCoreApplication>
#include <QDebug>

#include "Wallets/Monitor/MonitorManager.h"

using namespace Daitengu::Wallets;

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    MonitorManager manager;

    auto btcMonitor = std::make_unique<BlockchainMonitor>(
        ChainType::BITCOIN, ProviderType::MEMPOOL_SPACE);
    manager.registerMonitor(ChainType::BITCOIN, std::move(btcMonitor));

    QObject::connect(&manager, &MonitorManager::balanceUpdated,
        [](const QString& address, const QString& balance, ChainType chain) {
            spdlog::info("Balance updated for address {} on chain {}: {}",
                address.toStdString(), static_cast<int>(chain),
                balance.toStdString());
            qDebug() << "Address:" << address << "Balance:" << balance
                     << "Chain:" << static_cast<int>(chain);
        });

    QObject::connect(&manager, &MonitorManager::error,
        [](const QString& message, ChainType chain) {
            spdlog::error("Error on chain {}: {}", static_cast<int>(chain),
                message.toStdString());
            qDebug() << "Error:" << message
                     << "Chain:" << static_cast<int>(chain);
        });

    QString address = "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa";
    spdlog::info("Querying balance for address: {}", address.toStdString());

    manager.setAddress(address)
        .then([&manager]() { return manager.refreshBalance(); })
        .then([&app]() {
            spdlog::info("Balance query completed");
            app.quit();
        });

    return app.exec();
}
