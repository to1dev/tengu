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

#include <spdlog/spdlog.h>

namespace Daitengu::Clients::Hydra {

Hydra::Hydra(QObject* parent)
    : QObject(parent)
{
}

Hydra::~Hydra()
{
    stopAll();
}

void Hydra::addSource(std::unique_ptr<DataSource> source)
{
    connect(source.get(), &DataSource::dataUpdated, this, &Hydra::dataUpdated);
    connect(
        source.get(), &DataSource::errorOccurred, this, &Hydra::errorOccurred);
    sources_.push_back(std::move(source));
    spdlog::info("Hydra: Added data source: {}",
        sources_.back()->getName().toStdString());
}

void Hydra::startAll()
{
    for (const auto& source : sources_) {
        source->start();
    }
    spdlog::info("Hydra: Started all data sources");
}

void Hydra::stopAll()
{
    for (const auto& source : sources_) {
        source->stop();
    }
    spdlog::info("Hydra: Stopped all data sources");
}

QVariantMap Hydra::getData(const QString& sourceName) const
{
    for (const auto& source : sources_) {
        if (source->getName() == sourceName) {
            return source->getData();
        }
    }
    return QVariantMap();
}
}
