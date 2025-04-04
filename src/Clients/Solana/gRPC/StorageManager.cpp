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

#include "StorageManager.h"

namespace Daitengu::Clients::Solana::gRPC {

StorageManager::StorageManager(const std::string& dbPath)
{
    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::DB* db;
    rocksdb::Status status = rocksdb::DB::Open(options, dbPath, &db);
    if (!status.ok()) {
        spdlog::error("Failed to open RocksDB: {}", status.ToString());
        throw std::runtime_error("RocksDB open failed");
    }
    db_.reset(db);
}

StorageManager::~StorageManager() = default;

QCoro::Task<void> StorageManager::storeBatch(
    const std::vector<std::pair<std::string, std::string>>& batch)
{
    rocksdb::WriteBatch writeBatch;
    for (const auto& [key, value] : batch) {
        writeBatch.Put(key, value);
    }

    rocksdb::Status status = db_->Write(rocksdb::WriteOptions(), &writeBatch);
    if (!status.ok()) {
        spdlog::error("Failed to store batch: {}", status.ToString());
        throw std::runtime_error("Batch store failed");
    }

    co_return;
}

QCoro::Task<std::optional<std::string>> StorageManager::getTransaction(
    const std::string& key)
{
    std::string value;
    rocksdb::Status status = db_->Get(rocksdb::ReadOptions(), key, &value);
    if (status.ok())
        co_return value;

    co_return std::nullopt;
}
}
