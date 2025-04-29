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

#include <qcoro5/qcoro/QCoro>

#include "DataSource.h"

namespace Daitengu::Clients::Hydra {

class SolanaBlockHashDataSource : public DataSource {
    Q_OBJECT

public:
    explicit SolanaBlockHashDataSource(const QString& name,
        const QString& rpcUrl = QString(), int timeoutMs = 5000,
        QObject* parent = nullptr);
    ~SolanaBlockHashDataSource() override;

    void start() override;
    void stop() override;
    QVariantMap getData() const override;

    QString getName() const override
    {
        return name_;
    }

    void setRpcUrl(const QString& rpcUrl);

    void fetchBlockHash();

private:
    QCoro::Task<void> fetchBlockHashAsync();
    QString name_;
    QString rpcUrl_;
    int timeoutMs_;
    bool running_ { false };
    std::unique_ptr<QNetworkAccessManager> networkManager_;

    mutable QMutex dataMutex_;
    QVariantMap blockHashData_;
};
}
