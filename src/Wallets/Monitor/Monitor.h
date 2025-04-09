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

#include <memory>
#include <optional>
#include <unordered_map>

#include <QNetworkAccessManager>
#include <QObject>

#include <qcoro/QCoro>

#include "Wallets/Core/Types.h"

namespace Daitengu::Wallets {

class MonitorPrivate;
class Manager;

namespace {
    inline constexpr char MEMPOOL_API[] = "https://mempool.space/api/address/";
    inline constexpr int DEFAULT_INTERVAL = 60000;
}

class Monitor : public QObject {
    Q_OBJECT

public:
    struct BalanceResult {
        QString address;
        ChainType chain;
        double balance { 0.0 };
        bool success { false };
        QString errorMessage;
        QString dataSource;
    };

    explicit Monitor(QObject* parent = nullptr);
    ~Monitor() override;

    void setAddress(ChainType chain, const QString& address);
    std::pair<ChainType, QString> getAddress() const;
    void setRefreshInterval(int interval);
    void setPreferredApiSource(ChainType chain, const QString& apiEndpoint);
    void refresh();

    QCoro::Task<std::optional<BalanceResult>> fetchBalance(
        ChainType chain, const QString& address);

Q_SIGNALS:
    void balanceUpdated(const BalanceResult& result);
    void errorOccurred(const QString& error);
    void addressChanged(ChainType chain, const QString& address);

private:
    std::unique_ptr<MonitorPrivate> d_ptr;
    Q_DECLARE_PRIVATE(Monitor)
};
}
