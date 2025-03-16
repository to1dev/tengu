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

#include <QDebug>
#include <QMutexLocker>
#include <QThread>

#include "TransactionFetchManager.h"

namespace Daitengu::Clients::Solana {

TransactionFetchManager::TransactionFetchManager(QObject* parent)
    : QObject(parent)
{
    connect(
        &timer_, &QTimer::timeout, this, &TransactionFetchManager::onTimerTick);
}

TransactionFetchManager::~TransactionFetchManager()
{
    stop();
}

void TransactionFetchManager::setMaxRequestsPerSecond(int rps)
{
    if (rps < 1)
        rps = 1;
    maxRequestsPerSecond_ = rps;
    tokensPerMs_ = double(rps) / 1000.0; // RPS => tokens per ms
}

void TransactionFetchManager::setWorkerCount(int count)
{
    if (count < 1)
        count = 1;
    workerCount_ = count;
}

void TransactionFetchManager::setMaxQueueSize(int size)
{
    maxQueueSize_ = size;
}

void TransactionFetchManager::setTransactionFetcher(
    TransactionFetchWorker::TransactionFetcher fetcher)
{
    fetcher_ = fetcher;
}

bool TransactionFetchManager::enqueueSignature(const std::string& signature,
    int maxRetries, std::chrono::milliseconds initialBackoff,
    TxResultCallback cb)
{
    QMutexLocker lock(&queueMutex_);
    if (queue_.size() >= maxQueueSize_) {
        return false;
    }

    TxTask t;
    t.signature = signature;
    t.maxRetries = maxRetries;
    t.backoff = initialBackoff;
    t.cb = cb;

    queue_.enqueue(t);
    return true;
}

void TransactionFetchManager::start()
{
    if (running_)
        return;
    running_ = true;

    for (int i = 0; i < workerCount_; i++) {
        auto* w = new TransactionFetchWorker();
        w->setFetcher([this](const std::string& sig,
                          std::function<void(const json&, bool)> resultCb) {
            if (fetcher_) {
                fetcher_(sig, resultCb);
            } else {
                resultCb(json(), false);
            }
        });
        w->start();
        workers_.append(w);
    }

    timer_.start(100);
    lastRefill_ = std::chrono::steady_clock::now();
    qDebug() << "[TransactionFetchManager] started with" << workerCount_
             << "workers. RPS=" << maxRequestsPerSecond_;
}

void TransactionFetchManager::stop()
{
    if (!running_)
        return;
    running_ = false;
    timer_.stop();

    for (auto* w : workers_) {
        w->requestStop();
    }
    for (auto* w : workers_) {
        w->quit();
        w->wait();
        delete w;
    }
    workers_.clear();

    {
        QMutexLocker locker(&queueMutex_);
        queue_.clear();
    }
    qDebug() << "[TransactionFetchManager] stopped.";
}

void TransactionFetchManager::onTimerTick()
{
    refillTokens();

    for (int i = 0; i < workers_.size(); i++) {
        auto* w = workers_[i];
        if (!w->isRunning()) {
            w->deleteLater();
            auto* nw = new TransactionFetchWorker();
            nw->setFetcher([this](const std::string& sig,
                               std::function<void(const json&, bool)> rcb) {
                if (fetcher_) {
                    fetcher_(sig, rcb);
                } else {
                    rcb(json(), false);
                }
            });
            nw->start();
            workers_[i] = nw;
        }
    }

    for (auto* worker : workers_) {
        if (!worker->isRunning()) {
            continue;
        }
        if (currentTokens_ >= 1.0) {
            if (queue_.isEmpty())
                break;

            currentTokens_ -= 1.0;

            auto t = dequeueOne();
            TransactionFetchWorker::Task wtask;
            wtask.signature = t.signature;
            wtask.retryCount = 0;
            wtask.maxRetries = t.maxRetries;
            wtask.backoff = t.backoff;
            wtask.cb
                = [t](const std::string& sig, const json& tx, bool success) {
                      if (t.cb) {
                          t.cb(sig, tx, success);
                      }
                  };

            worker->setTask(wtask);
        }
    }
}

void TransactionFetchManager::refillTokens()
{
    auto now = std::chrono::steady_clock::now();
    double ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - lastRefill_)
                    .count();
    lastRefill_ = now;
    double delta = ms * tokensPerMs_;
    currentTokens_ += delta;
    if (currentTokens_ > (double)maxRequestsPerSecond_ * 2.0) {
        currentTokens_ = (double)maxRequestsPerSecond_ * 2.0;
    }
}

TransactionFetchManager::TxTask TransactionFetchManager::dequeueOne()
{
    QMutexLocker lock(&queueMutex_);
    auto t = queue_.dequeue();
    return t;
}
}
