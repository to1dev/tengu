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

#include "Database.h"

namespace Daitengu::Databases {

DatabaseContext::DatabaseContext(const QString& dataPath,
    const std::shared_ptr<const DatabaseConfig>& config)
    : storage_(std::make_unique<Storage>(initStorage(dataPath)))
    , config_(config)
{
    storage_->pragma.journal_mode(config_->journalMode);
    storage_->pragma.busy_timeout(config_->busyTimeoutMs);

    int currentVersion = getVersion(storage_.get());
    int targetVersion = config_->targetVersion;
    bool needsUpgrade = (currentVersion < targetVersion);

    /*QString dbFile = QDir::toNativeSeparators(
        QString("%1/%2.db").arg(dataPath).arg(DB_NAME));
    bool dbExists = QFile::exists(dbFile);*/

    // or dbExists
    bool shouldBackup
        = needsUpgrade && config_->backupBeforeUpdate && (currentVersion > 0);

    if (shouldBackup) {
        try {
            if (config_->useThreadedBackup) {
                createBackup_async(dataPath);
            } else {
                createBackup(dataPath);
            }
        } catch (const std::exception& e) {
            qWarning() << "Database backup before upgrade failed:" << e.what();
        }
    }

    if (needsUpgrade && config_->autoSyncSchema) {
        try {
            storage_->sync_schema(true);
        } catch (const std::exception& e) {
            qCritical() << "Schema sync failed:" << e.what();
            throw DatabaseError(DatabaseErrorType::schemaUpgradeFailed,
                std::string("Schema upgrade failed: ") + e.what());
        }
    }

    try {
        applyMigrations(currentVersion, targetVersion);
    } catch (const std::exception& e) {
        qCritical() << "Migration failed:" << e.what();
        throw DatabaseError(DatabaseErrorType::migrationFailed,
            std::string("Migration failed: ") + e.what());
    }

    qInfo() << "Database initialized successfully. Version:"
            << getVersion(storage_.get());
}

Storage* DatabaseContext::storage()
{
    return storage_.get();
}

int DatabaseContext::getVersion(Storage* storage)
{
    if (!storage->table_exists("migration")) {
        return 0;
    }

    auto records = storage->get_all<Migration>();
    return records.empty() ? 0 : records.front().version;
}

void DatabaseContext::setVersion(Storage* storage, int version)
{
    storage->replace(Migration { 1, version });
}

void DatabaseContext::applyMigrations(int currentVersion, int targetVersion)
{
    if (currentVersion < 1 && targetVersion >= 1) {
        qInfo() << "Applying migration to version 1";
        storage_->transaction([&] {
            storage_->replace(Migration { 1, 1 });
            return true;
        });
        currentVersion = 1;
    }

    if (currentVersion < 2 && targetVersion >= 2) {
        qInfo() << "Applying migration to version 2";
        storage_->transaction([&] {
            storage_->replace(Migration { 1, 2 });
            return true;
        });
        currentVersion = 2;
    }

    if (currentVersion != getVersion(storage_.get())) {
        setVersion(storage_.get(), currentVersion);
    }
}

void DatabaseContext::createBackup(const QString& dataPath)
{
    QString dbFile = QDir::toNativeSeparators(
        QString("%1/%2.db").arg(dataPath).arg(DB_NAME));

    QString backupDir = dataPath + "/db_backups";
    QDir().mkpath(backupDir);

    int currentVersion = getVersion(storage_.get());
    QString backupFile
        = QString("%1/%2_v%3_%4.db")
              .arg(backupDir)
              .arg(DB_NAME)
              .arg(currentVersion)
              .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

    QFile file(dbFile);
    if (file.exists()) {
        qInfo() << "Creating database backup before upgrade:" << backupFile;
        if (!file.copy(backupFile)) {
            qWarning() << "Failed to create database backup:"
                       << file.errorString();
        } else {
            cleanupOldBackups(backupDir, config_->maxBackupCount);
        }
    }
}

void DatabaseContext::createBackup_async(const QString& dataPath)
{
    QString dbFile = QDir::toNativeSeparators(
        QString("%1/%2.db").arg(dataPath).arg(DB_NAME));

    QString backupDir = dataPath + "/db_backups";
    QDir().mkpath(backupDir);

    int currentVersion = getVersion(storage_.get());
    QString backupFile
        = QString("%1/%2_v%3_%4.db")
              .arg(backupDir)
              .arg(DB_NAME)
              .arg(currentVersion)
              .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

    QEventLoop loop;
    bool backupSuccess = false;

    QThread* backupThread = new QThread();
    DatabaseBackupWorker* worker = new DatabaseBackupWorker(storage_.get());
    worker->moveToThread(backupThread);

    QObject::connect(
        backupThread, &QThread::finished, worker, &QObject::deleteLater);
    QObject::connect(
        worker, &DatabaseBackupWorker::backupCompleted, [&](const QString&) {
            backupSuccess = true;
            loop.quit();
        });
    QObject::connect(
        worker, &DatabaseBackupWorker::backupFailed, [&](const QString& error) {
            qWarning() << "Backup before upgrade failed:" << error;
            backupSuccess = false;
            loop.quit();
        });

    backupThread->start();

    QMetaObject::invokeMethod(worker, "performBackup", Qt::QueuedConnection,
        Q_ARG(QString, backupFile));

    QTimer::singleShot(30000, &loop, &QEventLoop::quit);
    loop.exec();

    backupThread->quit();
    backupThread->wait();
    backupThread->deleteLater();

    if (backupSuccess) {
        cleanupOldBackups(backupDir, config_->maxBackupCount);
    }
}

void DatabaseContext::cleanupOldBackups(const QString& backupDir, int maxCount)
{
    QDir dir(backupDir);
    QStringList filters;
    filters << QString("%1_*.db").arg(DB_NAME);
    dir.setNameFilters(filters);
    dir.setSorting(QDir::Time);

    QStringList backups = dir.entryList();
    if (backups.size() > maxCount) {
        for (int i = maxCount; i < backups.size(); ++i) {
            QFile::remove(dir.filePath(backups[i]));
        }
    }
}

WalletGroupRepo::WalletGroupRepo(Storage* storage)
    : storage_(storage)
{
}

DBErrorType WalletGroupRepo::before(const WalletGroup& group, bool update)
{
    return DBErrorType::none;
}

int WalletGroupRepo::insert(const WalletGroup& group)
{
    return storage_->insert(group);
}

void WalletGroupRepo::update(const WalletGroup& group)
{
    storage_->update(group);
}

void WalletGroupRepo::remove(int id)
{
    storage_->remove<WalletGroup>(id);
}

std::optional<WalletGroup> WalletGroupRepo::get(int id)
{
    if (auto group = storage_->get_pointer<WalletGroup>(id))
        return *group;
    return std::nullopt;
}

std::vector<WalletGroup> WalletGroupRepo::getAll()
{
    return storage_->get_all<WalletGroup>();
}

WalletRepo::WalletRepo(Storage* storage)
    : storage_(storage)
{
}

DBErrorType WalletRepo::before(const Wallet& wallet, bool update)
{
    int countName = storage_->count<Wallet>(
        where(c(&Wallet::nameHash) == wallet.nameHash));
    if (countName > 0) {
        return DBErrorType::haveName;
    }

    if (!update) {
        int countMnemonic = storage_->count<Wallet>(
            where(c(&Wallet::mnemonicHash) == wallet.mnemonicHash));
        if (countMnemonic > 0) {
            return DBErrorType::haveMnemonic;
        }
    }

    return DBErrorType::none;
}

int WalletRepo::insert(const Wallet& wallet)
{
    int id = storage_->insert(wallet);
    Q_EMIT inserted();
    std::lock_guard<std::mutex> lock(cbMutex_);
    for (const auto& cb : insertedCbs_)
        try {
            if (cb)
                cb();
        } catch (const std::exception& e) {
            std::cerr << "Callback failed: " << e.what() << std::endl;
        }

    return id;
}

void WalletRepo::update(const Wallet& wallet)
{
    storage_->update(wallet);
    Q_EMIT updated(wallet);
    std::lock_guard<std::mutex> lock(cbMutex_);
    for (const auto& cb : updatedCbs_)
        try {
            if (cb)
                cb(wallet);
        } catch (const std::exception& e) {
            std::cerr << "Callback failed: " << e.what() << std::endl;
        }
}

void WalletRepo::remove(int id)
{
    storage_->remove_all<Address>(where(c(&Address::walletId) == id));
    storage_->remove<Wallet>(id);
    Q_EMIT removed(id);
    std::lock_guard<std::mutex> lock(cbMutex_);
    for (const auto& cb : removedCbs_)
        try {
            if (cb)
                cb(id);
        } catch (const std::exception& e) {
            std::cerr << "Callback failed: " << e.what() << std::endl;
        }
}

std::optional<Wallet> WalletRepo::get(int id)
{
    if (auto wallet = storage_->get_pointer<Wallet>(id))
        return *wallet;
    return std::nullopt;
}

std::vector<Wallet> WalletRepo::getAll()
{
    auto result = safeExecute([&] {
        return storage_->get_all<Wallet>(order_by(&Wallet::groupType));
    });
    return result.value_or(std::vector<Wallet> {});

    // return storage_->get_all<Wallet>(order_by(&Wallet::groupType));
}

std::vector<Wallet> WalletRepo::getByGroup(int groupType)
{
    auto result = safeExecute([&] {
        return storage_->get_all<Wallet>(
            where(c(&Wallet::groupType) == groupType));
    });
    return result.value_or(std::vector<Wallet> {});

    // return storage_->get_all<Wallet>(where(c(&Wallet::groupType) ==
    // groupType));
}

void WalletRepo::addRemovedCallback(std::function<void(int)> cb)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    removedCbs_.push_back(std::move(cb));
}

void WalletRepo::addUpdatedCallback(std::function<void(const Wallet&)> cb)
{
    updatedCbs_.push_back(std::move(cb));
}

void WalletRepo::clearCallbacks()
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    insertedCbs_.clear();
    updatedCbs_.clear();
    removedCbs_.clear();
}

AddressRepo::AddressRepo(Storage* storage)
    : storage_(storage)
{
}

DBErrorType AddressRepo::before(const Address& address, bool update)
{
    int countName = storage_->count<Address>(
        where(c(&Address::nameHash) == address.nameHash)
        && c(&Address::walletId) == address.walletId);
    if (countName > 0) {
        return DBErrorType::haveName;
    }

    return DBErrorType::none;
}

bool AddressRepo::haveName(const Address& address)
{
    int countName = storage_->count<Address>(
        where(c(&Address::nameHash) == address.nameHash)
        && c(&Address::walletId) == address.walletId);

    if (countName > 0) {
        return true;
    }

    return false;
}

bool AddressRepo::haveAddress(const Address& address)
{
    int countName = storage_->count<Address>(
        where(c(&Address::addressHash) == address.addressHash)
        && c(&Address::walletId) == address.walletId);

    if (countName > 0) {
        return true;
    }

    return false;
}

int AddressRepo::insert(const Address& address)
{
    int id = storage_->insert(address);
    Q_EMIT inserted();
    std::lock_guard<std::mutex> lock(cbMutex_);
    for (const auto& cb : insertedCbs_)
        try {
            if (cb)
                cb();
        } catch (const std::exception& e) {
            std::cerr << "Callback failed: " << e.what() << std::endl;
        }

    return id;
}

void AddressRepo::update(const Address& address)
{
    storage_->update(address);
    Q_EMIT updated(address);
    std::lock_guard<std::mutex> lock(cbMutex_);
    for (const auto& cb : updatedCbs_)
        try {
            if (cb)
                cb(address);
        } catch (const std::exception& e) {
            std::cerr << "Callback failed: " << e.what() << std::endl;
        }
}

void AddressRepo::remove(int id)
{
    storage_->remove<Address>(id);
    Q_EMIT removed(id);
    std::lock_guard<std::mutex> lock(cbMutex_);
    for (const auto& cb : removedCbs_)
        try {
            if (cb)
                cb(id);
        } catch (const std::exception& e) {
            std::cerr << "Callback failed: " << e.what() << std::endl;
        }
}

std::optional<Address> AddressRepo::get(int id)
{
    if (auto addr = storage_->get_pointer<Address>(id))
        return *addr;
    return std::nullopt;
}

std::vector<Address> AddressRepo::getAllByWallet(int walletId)
{
    return storage_->get_all<Address>(where(c(&Address::walletId) == walletId));
}

void AddressRepo::addRemovedCallback(std::function<void(int)> cb)
{
    removedCbs_.push_back(std::move(cb));
}

void AddressRepo::addUpdatedCallback(std::function<void(const Address&)> cb)
{
    updatedCbs_.push_back(std::move(cb));
}

void AddressRepo::clearCallbacks()
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    insertedCbs_.clear();
    updatedCbs_.clear();
    removedCbs_.clear();
}

Database::Database(const QString& dataPath,
    const std::shared_ptr<const DatabaseConfig>& config)
    : context_(dataPath, config)
    , config_(config)
{
    storage_ = context_.storage();
    walletGroupRepo_ = std::make_unique<WalletGroupRepo>(storage_);
    walletRepo_ = std::make_unique<WalletRepo>(storage_);
    addressRepo_ = std::make_unique<AddressRepo>(storage_);

    if (config_->enableScheduledBackups) {
        startPeriodicBackups(config_->backupIntervalHours);
    }
}

Database::~Database()
{
    if (backupThread_) {
        backupThread_->quit();
        backupThread_->wait();
        backupThread_->deleteLater();
    }

    if (backupTimer_) {
        backupTimer_->stop();
        backupTimer_->deleteLater();
    }
}

IWalletGroupRepo* Database::walletGroupRepo()
{
    return walletGroupRepo_.get();
}

IWalletRepo* Database::walletRepo()
{
    return walletRepo_.get();
}

IAddressRepo* Database::addressRepo()
{
    return addressRepo_.get();
}

Storage* Database::storage()
{
    return storage_;
}

void Database::reset()
{
    storage_->remove_all<Subscription>();
    storage_->remove_all<Order>();
    storage_->remove_all<addressTag>();
    storage_->remove_all<Tag>();
    storage_->remove_all<Address>();
    storage_->remove_all<Wallet>();
    storage_->remove_all<Network>();
    storage_->remove_all<WalletGroup>();
    storage_->remove_all<Migration>();
    storage_->vacuum();
}

void Database::startPeriodicBackups(int intervalHours)
{
    if (intervalHours <= 0) {
        intervalHours = config_->backupIntervalHours;
        if (intervalHours <= 0)
            return;
    }

    if (config_->useThreadedBackup && !backupThread_) {
        backupThread_ = new QThread(this);
        backupWorker_ = new DatabaseBackupWorker(storage_);
        backupWorker_->moveToThread(backupThread_);

        QObject::connect(backupThread_, &QThread::finished, backupWorker_,
            &QObject::deleteLater);
        QObject::connect(this, &Database::requestBackup, backupWorker_,
            &DatabaseBackupWorker::performBackup);
        QObject::connect(backupWorker_, &DatabaseBackupWorker::backupCompleted,
            this, &Database::handleBackupCompleted);
        QObject::connect(backupWorker_, &DatabaseBackupWorker::backupFailed,
            this, &Database::handleBackupFailed);

        backupThread_->start();
    }

    if (lastBackupTime_.isNull()) {
        lastBackupTime_ = QDateTime::currentDateTime();
    }

    if (!backupTimer_) {
        backupTimer_ = new QTimer(this);
        QObject::connect(backupTimer_, &QTimer::timeout, this,
            &Database::checkScheduledBackup);
        backupTimer_->start(60 * 60 * 1000);
    }

    qInfo() << "Scheduled database backups enabled, interval:" << intervalHours
            << "hours";
}

void Database::checkScheduledBackup()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    int hoursSinceLastBackup = lastBackupTime_.secsTo(currentTime) / 3600;

    if (hoursSinceLastBackup >= config_->backupIntervalHours) {
        qInfo() << "Scheduled backup triggered, hours since last backup:"
                << hoursSinceLastBackup;
        performScheduledBackup();
    }
}

void Database::handleBackupCompleted(const QString& backupFilePath)
{
    lastBackupTime_ = QDateTime::currentDateTime();

    QString backupDir = QFileInfo(backupFilePath).dir().path();
    cleanupOldBackups(backupDir, config_->maxScheduledBackupCount);

    qInfo() << "Threaded backup completed:" << backupFilePath;

    Q_EMIT backupSucceeded(backupFilePath);
}

void Database::handleBackupFailed(const QString& errorMessage)
{
    qWarning() << "Threaded backup failed:" << errorMessage;

    Q_EMIT backupFailed(errorMessage);
}

void Database::performScheduledBackup()
{
    QString backupPath
        = QString::fromStdString(config_->dataPath) + "/scheduled_backups";
    QDir().mkpath(backupPath);

    QString backupFileName
        = QString("%1/%2_scheduled_%3.db")
              .arg(backupPath)
              .arg(DB_NAME)
              .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

    if (config_->useThreadedBackup && backupThread_) {
        Q_EMIT requestBackup(backupFileName);
    } else {
        try {
            storage_->backup_to(backupFileName.toStdString());
            lastBackupTime_ = QDateTime::currentDateTime();

            cleanupOldBackups(backupPath, config_->maxScheduledBackupCount);

            qInfo() << "Scheduled backup completed:" << backupFileName;
            Q_EMIT backupSucceeded(backupFileName);
        } catch (const std::exception& e) {
            QString errorMsg
                = QString("Scheduled backup failed: %1").arg(e.what());
            qWarning() << errorMsg;
            Q_EMIT backupFailed(errorMsg);
        }
    }
}

void Database::cleanupOldBackups(const QString& backupDir, int maxCount)
{
    QDir dir(backupDir);
    QStringList filters;
    filters << QString("%1_*.db").arg(DB_NAME);
    dir.setNameFilters(filters);
    dir.setSorting(QDir::Time);

    QStringList backups = dir.entryList();
    if (backups.size() > maxCount) {
        for (int i = maxCount; i < backups.size(); ++i) {
            QString filePath = dir.filePath(backups[i]);
            QFile::remove(filePath);
            qInfo() << "Removed old backup:" << filePath;
        }
    }
}

void Database::handleError(DatabaseErrorType type, const std::string& message)
{
    QString qMessage = QString::fromStdString(message);
    qCritical() << "[Database Error]" << qMessage;
    Q_EMIT databaseError(type, qMessage);

    if (type == DatabaseErrorType::corruptedDatabase) {
        Q_EMIT databaseCorrupted();
    }
}
}
