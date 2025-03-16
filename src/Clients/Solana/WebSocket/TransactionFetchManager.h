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

#include <atomic>
#include <chrono>
#include <functional>

#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QTimer>
#include <QVector>

#include "TransactionFetchWorker.h"

namespace Daitengu::Clients::Solana {

class TransactionFetchManager : public QObject {
    Q_OBJECT

public:
    using TxResultCallback
        = std::function<void(const std::string&, const json&, bool)>;

    explicit TransactionFetchManager(QObject* parent = nullptr);
    ~TransactionFetchManager();

    void setMaxRequestsPerSecond(int rps);

    void setWorkerCount(int count);

    void setMaxQueueSize(int size);

    bool enqueueSignature(const std::string& signature, int maxRetries,
        std::chrono::milliseconds initialBackoff, TxResultCallback cb);

    // (signature, [cb]) => (json, success)
    void setTransactionFetcher(
        TransactionFetchWorker::TransactionFetcher fetcher);

    void start();
    void stop();

private Q_SLOTS:
    void onTimerTick();

private:
    struct TxTask {
        std::string signature;
        int maxRetries;
        std::chrono::milliseconds backoff;
        TxResultCallback cb;
    };

    void refillTokens();
    double currentTokens_ = 0.0;
    double tokensPerMs_ = 0.01; // 10rps => 0.01 token/ms
    std::chrono::steady_clock::time_point lastRefill_;
    int maxRequestsPerSecond_ = 10;

    QMutex queueMutex_;
    QQueue<TxTask> queue_;
    int maxQueueSize_ = 10000;

    int workerCount_ = 4;
    QVector<TransactionFetchWorker*> workers_;

    TransactionFetchWorker::TransactionFetcher fetcher_ = nullptr;

    QTimer timer_;
    bool running_ = false;

    TxTask dequeueOne();
};
}
