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

#include <mutex>
#include <string>

#include <geyser/geyser.grpc.pb.h>

namespace solana {

class TransactionFilter {
public:
    virtual ~TransactionFilter() = default;
    virtual void processTransaction(const std::string& sourceId,
        const geyser::SubscribeUpdateTransaction& tx)
        = 0;
    virtual void updateConfig(const std::string& config) = 0;
    virtual std::string name() const = 0;

    uint64_t getProcessedCount() const
    {
        return processedCount_;
    }

    uint64_t getMatchedCount() const
    {
        return matchedCount_;
    }

protected:
    void incrementMatchCount()
    {
        ++matchedCount_;
    }

    void incrementProcessCount()
    {
        ++processedCount_;
    }

    mutable std::mutex mutex_;

private:
    uint64_t processedCount_ { 0 };
    uint64_t matchedCount_ { 0 };
};
}
