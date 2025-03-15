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

#include <concepts>
#include <functional>
#include <string_view>

#include <QObject>
#include <QSet>
#include <QString>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

#include "SolanaConnectionManager.h"

namespace Daitengu::Clients::Solana {

class SmartMoneyTracker : public QObject {
    Q_OBJECT

public:
    struct SmartMoneyCriteria {
        QSet<QString> smartAddresses;
        uint64_t minTransactionAmount = 1000000000;
        QSet<QString> trackedProgramIds;
        std::function<bool(const json&)> customFilter = nullptr;
    };

    explicit SmartMoneyTracker(QObject* parent = nullptr);
    ~SmartMoneyTracker();

    bool startTracking();
    void stopTracking();

    void setSmartMoneyCriteria(const SmartMoneyCriteria& criteria);

    void addSmartMoneyAddress(std::string_view address);
    void removeSmartMoneyAddress(std::string_view address);

    void addTrackedProgramId(std::string_view programId);
    void removeTrackedProgramId(std::string_view programId);

    void setMinTransactionAmount(uint64_t amount);

    template <typename Func>
        requires std::invocable<Func, const json&>
        && std::same_as<std::invoke_result_t<Func, const json&>, bool>
    void setCustomFilter(Func&& filter)
    {
        criteria_.customFilter = std::forward<Func>(filter);
    }

    const SmartMoneyCriteria& getCurrentCriteria() const;
    bool isTracking() const;

    void setName(std::string_view name);
    QString getName() const;

Q_SIGNALS:
    void smartMoneyTransactionDetected(const json& transaction);
    void trackingStatusChanged(bool isTracking);
    void error(const QString& errorMessage);

private:
    void processTransaction(const json& transaction);
    void processAccountUpdate(const json& accountData);

    bool isSmartMoneyTransaction(const json& transaction);

    bool checkSmartAddresses(const json& transaction) const;
    bool checkProgramIds(const json& transaction) const;
    bool checkTransactionAmount(const json& transaction) const;

    void reregisterListeners();
    void registerAccountListeners();

private:
    SmartMoneyCriteria criteria_;
    bool isTracking_ { false };

    int transactionListenerId_ { -1 };
    QMap<QString, int> accountListenerIds_;

    QString name_ { "SmartMoneyTracker" };
};

}
