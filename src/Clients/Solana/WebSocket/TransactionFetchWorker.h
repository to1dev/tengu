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

#include <chrono>
#include <functional>

#include <QThread>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace Daitengu::Clients::Solana {

class TransactionFetchWorker : public QThread {
    Q_OBJECT

public:
    using TxResultCallback = std::function<void(
        const std::string& signature, const json& tx, bool success)>;

    // fetcher: (signature, callback(tx, success))
    using TransactionFetcher = std::function<void(
        const std::string&, std::function<void(const json&, bool)>)>;

    explicit TransactionFetchWorker(QObject* parent = nullptr);
    ~TransactionFetchWorker() override;

    void requestStop();

    void setFetcher(TransactionFetcher fetcher);

    struct Task {
        std::string signature;
        int retryCount;
        int maxRetries;
        std::chrono::milliseconds backoff;
        TxResultCallback cb;
    };

    void setTask(const Task& task);

protected:
    void run() override;

private:
    bool stopRequested_ = false;

    Task currentTask_;
    bool hasTask_ = false;

    TransactionFetcher fetcher_;
};
}
