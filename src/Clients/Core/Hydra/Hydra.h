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

#include "DataSource.h"

#include <memory>
#include <vector>

#include <QObject>

namespace Daitengu::Clients {

class Hydra : public QObject {
    Q_OBJECT

public:
    explicit Hydra(QObject* parent = nullptr);
    ~Hydra();

    void addSource(std::unique_ptr<DataSource> source);
    void startAll();
    void stopAll();
    QVariantMap getData(const QString& sourceName) const;

Q_SIGNALS:
    void dataUpdated(const QString& sourceName, const QVariantMap& data);
    void errorOccurred(const QString& sourceName, const QString& error);

private:
    std::vector<std::unique_ptr<DataSource>> sources_;
};
}
