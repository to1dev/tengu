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
#include <QObject>

#include <spdlog/spdlog.h>

#include "Clients/Core/Hydra/Hydra.h"
#include "Clients/Core/Hydra/PriceDataSource.h"

using namespace Daitengu::Clients;

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    Hydra hydra;

    auto priceSource
        = std::make_unique<PriceDataSource>("CryptoPrices", 60, 5000);
    hydra.addSource(std::move(priceSource));

    QObject::connect(&hydra, &Hydra::dataUpdated,
        [](const QString& sourceName, const QVariantMap& data) {
            qDebug() << "Data updated for" << sourceName << ":" << data;
        });
    QObject::connect(&hydra, &Hydra::errorOccurred,
        [](const QString& sourceName, const QString& error) {
            qDebug() << "Error in" << sourceName << ":" << error;
        });

    hydra.startAll();

    return app.exec();
}
