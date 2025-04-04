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

#include <vector>

#include <rocksdb/db.h>

#include <spdlog/spdlog.h>

#include <qcoro/QCoro>

namespace Daitengu::Clients::Solana::gRPC {

class StorageManager {
public:
    explicit StorageManager(const std::string& dbPath);
    ~StorageManager();

    QCoro::Task<void> storeBatch(
        const std::vector<std::pair<std::string, std::string>>& batch);
    QCoro::Task<std::optional<std::string>> getTransaction(
        const std::string& key);

    virtual void backupData(const std::string& backupPath)
    {
    }

private:
    std::unique_ptr<rocksdb::DB> db_;
};
}
