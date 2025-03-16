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

#include <functional>
#include <vector>

#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "SolanaConnectionManager.h"
#include "TransactionFetchManager.h"

namespace Daitengu::Clients::Solana {

class SmartMoneyTracker : public QObject {
    Q_OBJECT

public:
    struct SmartMoneyCriteria {
        QSet<QString> smartAddresses;
        uint64_t minTransactionAmount = 1000000000ULL; // 1 SOL
        QSet<QString> trackedProgramIds;
        std::function<bool(const json&)> customFilter = nullptr;
    };

    explicit SmartMoneyTracker(QObject* parent = nullptr);
    ~SmartMoneyTracker();

    void setName(std::string_view name);
    QString getName() const;

    void setSmartMoneyCriteria(const SmartMoneyCriteria& criteria);
    const SmartMoneyCriteria& getCriteria() const;

    bool startTracking();
    void stopTracking();
    bool isTracking() const;

Q_SIGNALS:
    void smartMoneyTransactionDetected(const json& fullTx);
    void trackingStatusChanged(bool tracking);
    void error(const QString& err);

private:
    struct LogsSubscriptionBucket {
        int subscriptionId = -1;
        std::vector<QString> addresses;
    };

    void registerLogsSubscriptions();
    void unregisterLogsSubscriptions();

    void onLogsNotification(const json& notif, int bucketIndex);

    void onTransactionResult(const json& fullTx, bool success);

    bool isSmartMoneyTransaction(const json& fullTx) const;
    bool checkSmartAddresses(const json& fullTx) const;
    bool checkProgramIds(const json& fullTx) const;
    bool checkTransactionAmount(const json& fullTx) const;

private:
    SmartMoneyCriteria criteria_;
    bool tracking_ = false;
    QString name_ { "SmartMoneyTracker" };

    std::vector<LogsSubscriptionBucket> buckets_;
    static const size_t MAX_MENTIONS_PER_BUCKET = 500;

    TransactionFetchManager fetchManager_;
};
}
