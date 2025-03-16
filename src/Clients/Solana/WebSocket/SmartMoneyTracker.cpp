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

#include <algorithm>

#include <QDebug>

#include "SmartMoneyTracker.h"

namespace Daitengu::Clients::Solana {

SmartMoneyTracker::SmartMoneyTracker(QObject* parent)
    : QObject(parent)
{
    fetchManager_.setWorkerCount(4);
    fetchManager_.setMaxRequestsPerSecond(10); // 10 RPS
    fetchManager_.setMaxQueueSize(10000);

    fetchManager_.setTransactionFetcher(
        [&](const std::string& sig, std::function<void(const json&, bool)> cb) {
            auto manager = SolanaConnectionManager::instance();
            manager->requestTransactionDetails(sig, [cb](const json& tx) {
                if (tx.is_null() || !tx.contains("transaction")) {
                    cb(tx, false);
                } else {
                    cb(tx, true);
                }
            });
        });
}

SmartMoneyTracker::~SmartMoneyTracker()
{
    stopTracking();
}

void SmartMoneyTracker::setName(std::string_view name)
{
    name_ = QString::fromStdString(std::string(name));
}

QString SmartMoneyTracker::getName() const
{
    return name_;
}

void SmartMoneyTracker::setSmartMoneyCriteria(
    const SmartMoneyCriteria& criteria)
{
    criteria_ = criteria;
    if (tracking_) {
        stopTracking();
        startTracking();
    }
}

const SmartMoneyTracker::SmartMoneyCriteria&
SmartMoneyTracker::getCriteria() const
{
    return criteria_;
}

bool SmartMoneyTracker::startTracking()
{
    if (tracking_)
        return true;

    /*auto connMgr = SolanaConnectionManager::instance();
    if (!connMgr->isConnected()) {
        Q_EMIT error("[SmartMoneyTracker] ConnectionManager not connected");
        return false;
    }*/

    fetchManager_.start();

    registerLogsSubscriptions();
    tracking_ = true;
    Q_EMIT trackingStatusChanged(true);

    qDebug() << "[SmartMoneyTracker]" << name_
             << "started with addresses:" << criteria_.smartAddresses.size();
    return true;
}

void SmartMoneyTracker::stopTracking()
{
    if (!tracking_)
        return;

    unregisterLogsSubscriptions();
    fetchManager_.stop();

    tracking_ = false;
    Q_EMIT trackingStatusChanged(false);
    qDebug() << "[SmartMoneyTracker]" << name_ << "stopped tracking";
}

bool SmartMoneyTracker::isTracking() const
{
    return tracking_;
}

void SmartMoneyTracker::registerLogsSubscriptions()
{
    buckets_.clear();

    std::vector<QString> allAddrs;
    for (auto& a : criteria_.smartAddresses) {
        allAddrs.push_back(a);
    }

    size_t idx = 0;
    while (idx < allAddrs.size()) {
        size_t end = std::min(idx + MAX_MENTIONS_PER_BUCKET, allAddrs.size());
        LogsSubscriptionBucket bucket;
        for (size_t i = idx; i < end; i++) {
            bucket.addresses.push_back(allAddrs[i]);
        }
        buckets_.push_back(bucket);
        idx = end;
    }

    if (buckets_.empty()) {
        buckets_.push_back({});
    }

    auto connMgr = SolanaConnectionManager::instance();

    for (size_t i = 0; i < buckets_.size(); i++) {
        json filter;
        if (!buckets_[i].addresses.empty()) {
            json arr = json::array();
            for (auto& addr : buckets_[i].addresses) {
                arr.push_back(addr.toStdString());
            }
            filter["mentions"] = arr;
        } else {
            // means no addresses => "all"?
        }

        int subId = connMgr->registerLogsListener(
            [this, i](const json& notif) { onLogsNotification(notif, (int)i); },
            filter);
        buckets_[i].subscriptionId = subId;
    }
}

void SmartMoneyTracker::unregisterLogsSubscriptions()
{
    auto connMgr = SolanaConnectionManager::instance();
    for (auto& b : buckets_) {
        if (b.subscriptionId != -1) {
            connMgr->unregisterListener(b.subscriptionId);
            b.subscriptionId = -1;
        }
    }
    buckets_.clear();
}

void SmartMoneyTracker::onLogsNotification(const json& notif, int bucketIndex)
{
    if (!notif.contains("value"))
        return;
    const auto& val = notif["value"];
    if (!val.contains("signature"))
        return;

    std::string signature = val["signature"].get<std::string>();

    bool en = fetchManager_.enqueueSignature(signature,
        /*maxRetries*/ 3, std::chrono::milliseconds(200),
        [this](const std::string& sig, const json& tx, bool success) {
            onTransactionResult(tx, success);
        });

    if (!en) {
        Q_EMIT error("[SmartMoneyTracker] fetch queue full, discard signature");
    }
}

void SmartMoneyTracker::onTransactionResult(const json& fullTx, bool success)
{
    if (!success) {
        return;
    }
    if (isSmartMoneyTransaction(fullTx)) {
        Q_EMIT smartMoneyTransactionDetected(fullTx);
    }
}

bool SmartMoneyTracker::isSmartMoneyTransaction(const json& fullTx) const
{
    if (!checkSmartAddresses(fullTx))
        return false;

    if (!checkTransactionAmount(fullTx))
        return false;

    if (!criteria_.trackedProgramIds.isEmpty() && !checkProgramIds(fullTx))
        return false;

    if (criteria_.customFilter) {
        if (!criteria_.customFilter(fullTx))
            return false;
    }

    return true;
}

bool SmartMoneyTracker::checkSmartAddresses(const json& fullTx) const
{
    if (criteria_.smartAddresses.isEmpty()) {
        return false;
    }

    if (!fullTx.contains("transaction"))
        return false;
    auto& txobj = fullTx["transaction"];
    if (!txobj.contains("message") || !txobj["message"].is_object())
        return false;
    auto& msg = txobj["message"];
    if (!msg.contains("accountKeys") || !msg["accountKeys"].is_array())
        return false;

    auto& arr = msg["accountKeys"];
    for (auto& acc : arr) {
        if (!acc.is_string())
            continue;
        QString a = QString::fromStdString(acc.get<std::string>());
        if (criteria_.smartAddresses.contains(a)) {
            return true;
        }
    }
    return false;
}

bool SmartMoneyTracker::checkProgramIds(const json& fullTx) const
{
    if (!fullTx.contains("transaction"))
        return false;
    auto& txobj = fullTx["transaction"];
    if (!txobj.contains("message") || !txobj["message"].is_object())
        return false;
    auto& msg = txobj["message"];
    if (!msg.contains("instructions") || !msg["instructions"].is_array())
        return false;

    auto& inst = msg["instructions"];
    for (auto& one : inst) {
        if (!one.is_object())
            continue;
        if (one.contains("programId")) {
            QString pid
                = QString::fromStdString(one["programId"].get<std::string>());
            if (criteria_.trackedProgramIds.contains(pid)) {
                return true;
            }
        }
    }
    return false;
}

bool SmartMoneyTracker::checkTransactionAmount(const json& fullTx) const
{
    if (criteria_.minTransactionAmount == 0)
        return true;
    if (!fullTx.contains("meta"))
        return false;
    auto& meta = fullTx["meta"];
    if (!meta.contains("preBalances") || !meta.contains("postBalances"))
        return false;
    auto& pre = meta["preBalances"];
    auto& post = meta["postBalances"];
    if (!pre.is_array() || !post.is_array())
        return false;

    size_t n = std::min(pre.size(), post.size());
    uint64_t maxChange = 0;
    for (size_t i = 0; i < n; i++) {
        if (!pre[i].is_number() || !post[i].is_number())
            continue;
        uint64_t p = pre[i].get<uint64_t>();
        uint64_t q = post[i].get<uint64_t>();
        uint64_t diff = (p > q) ? (p - q) : (q - p);
        if (diff > maxChange) {
            maxChange = diff;
        }
    }
    return (maxChange >= criteria_.minTransactionAmount);
}
}
