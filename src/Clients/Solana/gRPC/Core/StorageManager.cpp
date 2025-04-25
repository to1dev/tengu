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

#include "StorageManager.hpp"

#include <chrono>

#include <rocksdb/cache.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/table.h>
#include <rocksdb/utilities/backup_engine.h>

#include "Consts.h"
#include "Utils/PathUtils.hpp"

using namespace Daitengu::Core;
using namespace Daitengu::Utils;

#include "../Utils/Logger.hpp"

namespace solana {

class StorageWorker {
public:
    StorageWorker(rocksdb::DB* db)
        : db_(db)
    {
        worker_ = std::jthread([this](std::stop_token stoken) { run(stoken); });
    }

    ~StorageWorker()
    {
        stop();
    }

    void enqueueBatch(
        const std::vector<std::pair<std::string, std::string>>& batch)
    {
        if (batch.empty())
            return;
        std::lock_guard lock(mutex_);
        tasks_.push({ TaskType::BATCH, batch, "", "" });
        condition_.notify_one();
    }

    void enqueueGetRequest(const std::string& key)
    {
        std::lock_guard lock(mutex_);
        tasks_.push({ TaskType::GET, {}, key, "" });
        condition_.notify_one();
    }

    void enqueueBackupRequest(const std::string& backupPath)
    {
        std::lock_guard lock(mutex_);
        tasks_.push({ TaskType::BACKUP, {}, "", backupPath });
        condition_.notify_one();
    }

    uint64_t getTotalStoredTransactions() const
    {
        return totalStoredTransactions_;
    }

    uint64_t getTotalBatches() const
    {
        return totalBatches_;
    }

private:
    enum class TaskType { BATCH, GET, BACKUP };

    struct StorageTask {
        TaskType type;
        std::vector<std::pair<std::string, std::string>> batch;
        std::string key;
        std::string backupPath;
    };

    void stop()
    {
        {
            std::lock_guard lock(mutex_);
            shouldRun_ = false;
            condition_.notify_all();
        }
        worker_.request_stop();
        worker_.join();
    }

    void run(std::stop_token stoken)
    {
        Logger::getLogger()->info("Storage worker started");
        while (!stoken.stop_requested()) {
            StorageTask task;
            {
                std::unique_lock lock(mutex_);
                condition_.wait(lock, [this, &stoken] {
                    return !tasks_.empty() || !shouldRun_
                        || stoken.stop_requested();
                });
                if (!shouldRun_ && tasks_.empty())
                    break;
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            processTask(task);
        }
        Logger::getLogger()->info("Storage worker stopped");
    }

    void processTask(const StorageTask& task)
    {
        try {
            switch (task.type) {
            case TaskType::BATCH:
                processBatch(task.batch);
                break;
            case TaskType::GET:
                processGetRequest(task.key);
                break;
            case TaskType::BACKUP:
                processBackupRequest(task.backupPath);
                break;
            }
        } catch (const std::exception& e) {
            Logger::getLogger()->error("Storage task error: {}", e.what());
        }
    }

    void processBatch(
        const std::vector<std::pair<std::string, std::string>>& batch)
    {
        rocksdb::WriteBatch writeBatch;
        for (const auto& [key, value] : batch) {
            writeBatch.Put(key, value);
        }
        rocksdb::WriteOptions options;
        options.sync = false;
        options.disableWAL = false;
        rocksdb::Status status = db_->Write(options, &writeBatch);
        if (!status.ok()) {
            Logger::getLogger()->error(
                "Failed to store batch: {}", status.ToString());
            return;
        }
        totalStoredTransactions_ += batch.size();
        ++totalBatches_;
        Logger::getLogger()->debug(
            "Stored batch of {} transactions", batch.size());
    }

    void processGetRequest(const std::string& key)
    {
        std::string value;
        rocksdb::ReadOptions options;
        rocksdb::Status status = db_->Get(options, key, &value);
        if (status.ok()) {
            // Signal emission handled in StorageManager
        } else {
            Logger::getLogger()->error(
                "Error retrieving transaction {}: {}", key, status.ToString());
        }
    }

    void processBackupRequest(const std::string& backupPath)
    {
        rocksdb::BackupEngineOptions backupOptions(backupPath);
        backupOptions.sync = true;
        rocksdb::BackupEngine* backupEngine;
        rocksdb::Status status = rocksdb::BackupEngine::Open(
            rocksdb::Env::Default(), backupOptions, &backupEngine);
        std::unique_ptr<rocksdb::BackupEngine> backupEnginePtr(backupEngine);
        if (!status.ok()) {
            Logger::getLogger()->error(
                "Failed to create backup engine: {}", status.ToString());
            return;
        }
        status = backupEnginePtr->CreateNewBackup(db_);
        if (!status.ok()) {
            Logger::getLogger()->error(
                "Failed to create backup: {}", status.ToString());
            return;
        }
        Logger::getLogger()->info("Backup completed to {}", backupPath);
    }

    rocksdb::DB* db_;
    std::jthread worker_;
    std::queue<StorageTask> tasks_;
    std::mutex mutex_;
    std::condition_variable condition_;
    bool shouldRun_ { true };
    uint64_t totalStoredTransactions_ { 0 };
    uint64_t totalBatches_ { 0 };
};

StorageManager::StorageManager(const std::string& dbPath, QObject* parent)
    : QObject(parent)
    , dataPath_(PathUtils::getAppDataPath(COMPANY) / NAME)
{
    initializeDatabase(dbPath);
    worker_ = std::make_unique<StorageWorker>(db_.get());
}

StorageManager::~StorageManager() = default;

void StorageManager::initializeDatabase(const std::string& dbPath)
{
    fs::path fullDbPath = dataPath_ / dbPath;
    fs::create_directories(fullDbPath);
    dbOptions_.create_if_missing = true;
    dbOptions_.max_open_files = 1000;
    dbOptions_.compaction_style = rocksdb::kCompactionStyleLevel;
    dbOptions_.compression = rocksdb::kLZ4Compression;

    auto blockCache = rocksdb::NewLRUCache(1024 * 1024 * 1024); // 1GB

    rocksdb::BlockBasedTableOptions tableOptions;
    tableOptions.block_cache = blockCache;
    tableOptions.filter_policy.reset(rocksdb::NewBloomFilterPolicy(10, false));
    dbOptions_.table_factory.reset(
        rocksdb::NewBlockBasedTableFactory(tableOptions));
    dbOptions_.write_buffer_size = 128 * 1024 * 1024; // 128MB

    rocksdb::DB* db;
    rocksdb::Status status
        = rocksdb::DB::Open(dbOptions_, fullDbPath.string(), &db);

    if (!status.ok()) {
        throw std::runtime_error("RocksDB open failed: " + status.ToString());
    }

    db_.reset(db);
    Logger::getLogger()->info("RocksDB opened at {}", fullDbPath.string());
}

void StorageManager::storeBatch(
    const std::vector<std::pair<std::string, std::string>>& batch)
{
    worker_->enqueueBatch(batch);
}

void StorageManager::getTransaction(const std::string& key)
{
    worker_->enqueueGetRequest(key);
}

void StorageManager::backupData(const std::string& backupPath)
{
    worker_->enqueueBackupRequest(backupPath);
}

uint64_t StorageManager::getTotalStoredTransactions() const
{
    return worker_->getTotalStoredTransactions();
}

uint64_t StorageManager::getTotalBatches() const
{
    return worker_->getTotalBatches();
}

void StorageManager::optimizeDb()
{
    rocksdb::CompactRangeOptions options;
    options.bottommost_level_compaction
        = rocksdb::BottommostLevelCompaction::kForce;
    db_->CompactRange(options, nullptr, nullptr);
    Logger::getLogger()->info("Database optimization completed");
}
}
