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

#include <QObject>
#include <QVariantMap>

namespace Daitengu::Clients::Hydra {

class DataSource : public QObject {
    Q_OBJECT

public:
    explicit DataSource(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    virtual ~DataSource() = default;

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual QVariantMap getData() const = 0;
    virtual QString getName() const = 0;

Q_SIGNALS:
    void dataUpdated(const QString& sourceName, const QVariantMap& data);
    void errorOccurred(const QString& sourceName, const QString& error);
};
}
