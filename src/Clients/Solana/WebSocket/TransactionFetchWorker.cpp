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
#include <QElapsedTimer>

#include "TransactionFetchWorker.h"

namespace Daitengu::Clients::Solana {

TransactionFetchWorker::TransactionFetchWorker(QObject* parent)
    : QThread(parent)
{
}

TransactionFetchWorker::~TransactionFetchWorker()
{
}

void TransactionFetchWorker::requestStop()
{
    stopRequested_ = true;
}

void TransactionFetchWorker::setFetcher(TransactionFetcher fetcher)
{
    fetcher_ = fetcher;
}

void TransactionFetchWorker::setTask(const Task& task)
{
    currentTask_ = task;
    hasTask_ = true;
}

void TransactionFetchWorker::run()
{
    /*qDebug() << "[TransactionFetchWorker] Thread start:"
             << QThread::currentThreadId();*/

    if (!hasTask_) {
        //qDebug() << "[TransactionFetchWorker] No task, exit.";
        return;
    }

    Task task = currentTask_;
    int attempt = task.retryCount;

    while (!stopRequested_ && attempt <= task.maxRetries) {
        bool ok = false;
        json result;

        if (fetcher_) {
            fetcher_(task.signature, [&](const json& tx, bool success) {
                ok = success;
                result = tx;
            });
        } else {
            break;
        }

        if (ok) {
            if (task.cb) {
                task.cb(task.signature, result, true);
            }
            break;
        } else {
            if (attempt < task.maxRetries) {
                msleep(task.backoff.count());
            } else {
                if (task.cb) {
                    task.cb(task.signature, json(), false);
                }
            }
        }
        attempt++;
    }

    /*qDebug() << "[TransactionFetchWorker] Thread exit:"
             << QThread::currentThreadId();*/
}
}
