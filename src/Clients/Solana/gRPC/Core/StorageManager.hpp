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

#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <vector>

#include <QObject>

#include <rocksdb/db.h>

namespace solana {

class StorageWorker;

class StorageManager : public QObject {
    Q_OBJECT

public:
    explicit StorageManager(
        const std::string& dbPath, QObject* parent = nullptr);
    ~StorageManager();

    void storeBatch(
        const std::vector<std::pair<std::string, std::string>>& batch);
    void getTransaction(const std::string& key);
    void backupData(const std::string& backupPath);
    uint64_t getTotalStoredTransactions() const;
    uint64_t getTotalBatches() const;
    void optimizeDb();

Q_SIGNALS:
    void transactionRetrieved(
        const std::string& key, std::optional<std::string> value);
    void backupCompleted(
        const std::string& path, bool success, const QString& message);
    void storageError(const QString& errorMessage);

private:
    void initializeDatabase(const std::string& dbPath);
    std::unique_ptr<rocksdb::DB> db_;
    std::filesystem::path dataPath_;
    std::unique_ptr<StorageWorker> worker_;
    rocksdb::Options dbOptions_;
};
}
