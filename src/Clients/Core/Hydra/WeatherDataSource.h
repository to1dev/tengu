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

#include <QMutex>
#include <QNetworkAccessManager>
#include <QString>
#include <QTimer>

#include <qcoro5/qcoro/QCoro>

#include "DataSource.h"

namespace Daitengu::Clients::Hydra {

class WeatherDataSource : public DataSource {
    Q_OBJECT

public:
    explicit WeatherDataSource(const QString& name, const QString& apiKey,
        const QString& city, int interval, int timeoutMs,
        QObject* parent = nullptr);
    ~WeatherDataSource() override;

    void start() override;
    void stop() override;
    QVariantMap getData() const override;

    QString getName() const override
    {
        return name_;
    }

private:
    QCoro::Task<void> updateWeather();
    QString name_;
    QString apiKey_;
    QString city_;
    bool running_ { false };
    int interval_;
    int timeoutMs_;
    std::unique_ptr<QNetworkAccessManager> networkManager_;
    QTimer* timer_ { nullptr };

    mutable QMutex dataMutex_;
    QVariantMap weatherData_;
};
}
