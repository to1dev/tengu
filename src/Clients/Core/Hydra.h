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

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QStringList>

#include <qcoro/QCoro>
#include <qcoro/QCoroNetworkReply>

class QNetworkAccessManager;

namespace Daitengu::Clients {

class Hydra : public QObject {
    Q_OBJECT
public:
    explicit Hydra(
        QObject* parent = nullptr, int interval = 60, int timeoutMs = 5000);
    ~Hydra();

    void start();
    void stop();
    void setTickerList(const QStringList& tickers);
    void setApiUrl(const QString& apiUrl);

    double getPrice(
        const QString& ticker, const QString& currency = "USD") const;
    QMap<QString, double> getPrices() const;

Q_SIGNALS:
    void pricesUpdated(const QMap<QString, double>& prices);
    void errorOccurred(const QString& error);

private:
    QCoro::Task<void> updatePrices();
    bool running_ { false };
    int interval_;
    int timeoutMs_;
    QString apiUrl_;
    QStringList tickerList_;
    std::unique_ptr<QNetworkAccessManager> networkManager_;
    QTimer* timer_ { nullptr };

    mutable QMutex priceMutex_;
    QMap<QString, double> prices_;
};
}
