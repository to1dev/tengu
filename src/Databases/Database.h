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

#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QString>
#include <QThread>
#include <QTimer>

#include "sqlite_orm/sqlite_orm.h"

#include "Wallets/Core/Errors.hpp"
#include "Wallets/Core/Types.h"

using namespace sqlite_orm;

using namespace Daitengu::Wallets;

namespace Daitengu::Databases {

inline constexpr char DB_NAME[] = "tengu";

struct Migration {
    int id = 0;
    int version = 0;
};

enum class WalletGroupType {
    User = 0,
    Import,
    Smart,
    Vault,
};

enum class DatabaseErrorType {
    none = 0,
    corruptedDatabase,
    migrationFailed,
    schemaUpgradeFailed,
    backupFailed,
    restoreFailed,
    connectionLost,
    transactionFailed,
    constraintViolation,
    general
};

enum class DBErrorType {
    none = 0,
    haveName,
    haveMnemonic,
};

enum class OrderType {
    Market = 0,
    Limit = 1,
    StopLoss = 2,
    TakeProfit = 3,
    TrailingStopLoss = 4,
    TrailingTakeProfit = 5,
};

enum class OrderStatus {
    Pending = 0,
    PartiallyFilled = 1,
    Filled = 2,
    Cancelled = 3,
    Rejected = 4,
};

struct WalletGroup {
    int id = 0;
    int type = 0;
    std::string name = "";
};

struct Network {
    int id = 0;
    int chainType = 0;
    int chainId = 0;
    std::string name = "";
    std::string ticker = "";
};

struct Wallet {
    int id = 0;
    int type = static_cast<int>(WalletType::Unknown);
    int groupType = static_cast<int>(WalletGroupType::User);
    int chainType = static_cast<int>(ChainType::UNKNOWN);
    int networkType = static_cast<int>(NetworkType::UNKNOWN);
    std::string hash = "";
    std::string name = "";
    std::string nameHash = "";
    std::string mnemonic = "";
    std::string mnemonicHash = "";
    std::string passphrase = "";
    std::string masterPrivateKey = "";
    std::string extendedPublicKey = "";
    std::string description = "";
};

struct Address {
    int id = 0;
    int type = 0;
    int walletId = 0;
    int index = 0;
    std::string hash = "";
    std::string name = "";
    std::string nameHash = "";
    std::string address = "";
    std::string addressHash = "";
    std::string derivationPath = "";
    std::string privateKey = "";
    std::string publicKey = "";
    std::string description = "";
};

struct Tag {
    int id = 0;
    std::string name = 0;
    std::string description = "";
    long long createTimestamp = 0;
    long long updateTimestamp = 0;
};

struct addressTag {
    int id = 0;
    int addressId = 0;
    int tagId = 0;
    long long timestamp = 0;
};

struct Order {
    int id = 0;
    int walletId = 0;
    std::string symbol = "";
    std::string side = "";
    double quantity = 0.0;
    double price = 0.0;
    double stopPrice = 0.0;
    int orderType = 0;
    int status = 0;
    double executedQuantity = 0.0;
    double avgExecutionPrice = 0.0;
    long long createTimestamp = 0;
    long long updateTimestamp = 0;
    bool trailingStopEnabled = false;
    double trailingStopOffset = 0.0;
    double currentTrailingStopPrice = 0.0;
    bool trailingTakeProfitEnabled = false;
    double trailingTakeProfitOffset = 0.0;
    double currentTrailingTakeProfitPrice = 0.0;
};

struct Subscription {
    int id = 0;
    int walletId = 0;
    bool subscribed = false;
    bool followed = false;
    bool pinned = false;
    long long timestamp = 0;
};

struct DatabaseConfig {
    std::string dataPath;
    journal_mode journalMode = journal_mode::WAL;
    int syncMode = 1;
    int cacheSize = 2000;
    int busyTimeoutMs = 3000;
    bool enableForeignKeys = true;
    bool autoSyncSchema = true;
    int targetVersion = 1;
    bool backupBeforeUpdate = true;
    bool useThreadedBackup = false;
    int maxBackupCount = 5;
    bool enableScheduledBackups = false;
    int backupIntervalHours = 24;
    int maxScheduledBackupCount = 7;
    std::vector<std::string> initQueries;
};

class DatabaseError : public std::exception {
public:
    DatabaseError(DatabaseErrorType type, const std::string& message)
        : type_(type)
        , message_(message)
    {
    }

    DatabaseErrorType type() const
    {
        return type_;
    }

    const char* what() const noexcept override
    {
        return message_.c_str();
    }

private:
    DatabaseErrorType type_;
    std::string message_;
};

using Record = std::pair<Wallet, Address>;

inline auto initStorage(const QString& dataPath)
{
    QString filePath = QDir::toNativeSeparators(
        QString("%1/%2.db").arg(dataPath).arg(DB_NAME));

    return make_storage(filePath.toStdString(),
        make_table("migration",
            make_column("id", &Migration::id, primary_key()),
            make_column("version", &Migration::version)),

        make_table("groups",
            make_column("id", &WalletGroup::id, primary_key().autoincrement()),
            make_column("type", &WalletGroup::type),
            make_column("name", &WalletGroup::name)),

        make_table("networks",
            make_column("id", &Network::id, primary_key().autoincrement()),
            make_column("chainType", &Network::chainType),
            make_column("chainId", &Network::chainId),
            make_column("name", &Network::name),
            make_column("ticker", &Network::ticker)),

        make_table("wallets",
            make_column("id", &Wallet::id, primary_key().autoincrement()),
            make_column("type", &Wallet::type),
            make_column("groupType", &Wallet::groupType),
            make_column("chainType", &Wallet::chainType),
            make_column("networkType", &Wallet::networkType),
            make_column("hash", &Wallet::hash),
            make_column("name", &Wallet::name),
            make_column("nameHash", &Wallet::nameHash),
            make_column("mnemonic", &Wallet::mnemonic),
            make_column("mnemonicHash", &Wallet::mnemonicHash),
            make_column("passphrase", &Wallet::passphrase),
            make_column("masterPrivateKey", &Wallet::masterPrivateKey),
            make_column("extendedPublicKey", &Wallet::extendedPublicKey),
            make_column("description", &Wallet::description)),

        make_table("addresses",
            make_column("id", &Address::id, primary_key().autoincrement()),
            make_column("type", &Address::type),
            make_column("walletId", &Address::walletId),
            make_column("index", &Address::index),
            make_column("hash", &Address::hash),
            make_column("name", &Address::name),
            make_column("nameHash", &Address::nameHash),
            make_column("address", &Address::address),
            make_column("addressHash", &Address::addressHash),
            make_column("derivationPath", &Address::derivationPath),
            make_column("privateKey", &Address::privateKey),
            make_column("publicKey", &Address::publicKey),
            make_column("description", &Address::description)),

        make_table("tags",
            make_column("id", &Tag::id, primary_key().autoincrement()),
            make_column("name", &Tag::name),
            make_column("description", &Tag::description),
            make_column("createTimestamp", &Tag::createTimestamp),
            make_column("updateTimestamp", &Tag::updateTimestamp)),

        make_table("address_tags",
            make_column("id", &addressTag::id, primary_key().autoincrement()),
            make_column("addressId", &addressTag::addressId),
            make_column("tagId", &addressTag::tagId),
            make_column("timestamp", &addressTag::timestamp)),

        make_table("orders",
            make_column("id", &Order::id, primary_key().autoincrement()),
            make_column("walletId", &Order::walletId),
            make_column("symbol", &Order::symbol),
            make_column("side", &Order::side),
            make_column("quantity", &Order::quantity),
            make_column("price", &Order::price),
            make_column("stopPrice", &Order::stopPrice),
            make_column("orderType", &Order::orderType),
            make_column("executedQuantity", &Order::executedQuantity),
            make_column("avgExecutionPrice", &Order::avgExecutionPrice),
            make_column("createTimestamp", &Order::createTimestamp),
            make_column("updateTimestamp", &Order::updateTimestamp),
            make_column("trailingStopEnabled", &Order::trailingStopEnabled),
            make_column("trailingStopOffset", &Order::trailingStopOffset),
            make_column(
                "currentTrailingStopPrice", &Order::currentTrailingStopPrice),
            make_column(
                "trailingTakeProfitEnabled", &Order::trailingTakeProfitEnabled),
            make_column(
                "trailingTakeProfitOffset", &Order::trailingTakeProfitOffset),
            make_column("currentTrailingTakeProfitPrice",
                &Order::currentTrailingTakeProfitPrice)),

        make_table("subscriptions",
            make_column("id", &Subscription::id, primary_key().autoincrement()),
            make_column("walletId", &Subscription::walletId),
            make_column("subscribed", &Subscription::subscribed),
            make_column("followed", &Subscription::followed),
            make_column("pinned", &Subscription::pinned),
            make_column("timestamp", &Subscription::timestamp))

    );
}

using Storage = decltype(initStorage(""));

template <typename Func>
auto safeExecute(Func&& func) -> std::optional<decltype(func())>
{
    try {
        return func();
    } catch (const DatabaseError& e) {
        qCritical() << "[Database] Specific error:" << e.what()
                    << "Type:" << static_cast<int>(e.type());
        return std::nullopt;
    } catch (const std::exception& e) {
        qCritical() << "[Database] General error:" << e.what();
        return std::nullopt;
    } catch (...) {
        qCritical() << "[Database] Unknown error occurred";
        return std::nullopt;
    }
}

class DatabaseBackupWorker : public QObject {
    Q_OBJECT

public:
    DatabaseBackupWorker(Storage* storage)
        : storage_(storage)
    {
    }

public Q_SLOTS:

    void performBackup(const QString& backupFilePath)
    {
        try {
            storage_->backup_to(backupFilePath.toStdString());
            Q_EMIT backupCompleted(backupFilePath);
        } catch (const std::exception& e) {
            Q_EMIT backupFailed(QString("Backup failed: %1").arg(e.what()));
        }
    }

Q_SIGNALS:
    void backupCompleted(const QString& backupFilePath);
    void backupFailed(const QString& errorMessage);

private:
    Storage* storage_;
};

class DatabaseContext {
public:
    DatabaseContext(const QString& dataPath,
        const std::shared_ptr<const DatabaseConfig>& config);

    Storage* storage();

private:
    int getVersion(Storage* storage);
    void setVersion(Storage* storage, int version);

    void applyMigrations(int currentVersion, int targetVersion);

    void createBackup(const QString& dataPath);
    void createBackup_async(const QString& dataPath);
    void cleanupOldBackups(const QString& backupDir, int maxCount);

private:
    std::unique_ptr<Storage> storage_;
    std::shared_ptr<const DatabaseConfig> config_;
};

class IWalletGroupRepo {
public:
    virtual ~IWalletGroupRepo() = default;
    virtual DBErrorType before(const WalletGroup& group, bool update = false)
        = 0;
    virtual int insert(const WalletGroup& group) = 0;
    virtual void update(const WalletGroup& group) = 0;
    virtual void remove(int id) = 0;
    virtual std::optional<WalletGroup> get(int id) = 0;
    virtual std::vector<WalletGroup> getAll() = 0;
};

class WalletGroupRepo : public IWalletGroupRepo {
public:
    WalletGroupRepo(Storage* storage);
    DBErrorType before(const WalletGroup& group, bool update) override;
    int insert(const WalletGroup& group) override;
    void update(const WalletGroup& group) override;
    void remove(int id) override;
    std::optional<WalletGroup> get(int id) override;
    std::vector<WalletGroup> getAll() override;

private:
    Storage* storage_;
};

class IWalletRepo : public QObject {
    Q_OBJECT

public:
    virtual ~IWalletRepo() = default;
    virtual DBErrorType before(const Wallet& group, bool update = false) = 0;
    virtual int insert(const Wallet& wallet) = 0;
    virtual void update(const Wallet& wallet) = 0;
    virtual void remove(int id) = 0;
    virtual std::optional<Wallet> get(int id) = 0;
    virtual std::vector<Wallet> getAll() = 0;
    virtual std::vector<Wallet> getByGroup(int groupType) = 0;

    virtual void addRemovedCallback(std::function<void(int)> cb) = 0;
    virtual void addUpdatedCallback(std::function<void(const Wallet&)> cb) = 0;

Q_SIGNALS:
    void inserted();
    void updated(const Wallet& wallet);
    void removed(int id);
};

class WalletRepo : public IWalletRepo {
public:
    WalletRepo(Storage* storage);
    DBErrorType before(const Wallet& wallet, bool update) override;
    int insert(const Wallet& wallet) override;
    void update(const Wallet& wallet) override;
    void remove(int id) override;
    std::optional<Wallet> get(int id) override;
    std::vector<Wallet> getAll() override;
    std::vector<Wallet> getByGroup(int groupType) override;

    void addRemovedCallback(std::function<void(int)> cb);
    void addUpdatedCallback(std::function<void(const Wallet&)> cb);
    void clearCallbacks();

private:
    Storage* storage_;

    std::mutex cbMutex_;
    std::vector<std::function<void()>> insertedCbs_;
    std::vector<std::function<void(const Wallet&)>> updatedCbs_;
    std::vector<std::function<void(int)>> removedCbs_;
};

class IAddressRepo : public QObject {
    Q_OBJECT

public:
    virtual ~IAddressRepo() = default;
    virtual DBErrorType before(const Address& group, bool update = false) = 0;
    virtual int insert(const Address& address) = 0;
    virtual void update(const Address& address) = 0;
    virtual void remove(int id) = 0;
    virtual std::optional<Address> get(int id) = 0;
    virtual std::vector<Address> getAllByWallet(int walletId) = 0;

    virtual void addRemovedCallback(std::function<void(int)> cb) = 0;
    virtual void addUpdatedCallback(std::function<void(const Address&)> cb) = 0;

Q_SIGNALS:
    void inserted();
    void updated(const Address& address);
    void removed(int id);
};

class AddressRepo : public IAddressRepo {
public:
    AddressRepo(Storage* storage);
    DBErrorType before(const Address& address, bool update) override;
    int insert(const Address& address) override;
    void update(const Address& address) override;
    void remove(int id) override;
    std::optional<Address> get(int id) override;
    std::vector<Address> getAllByWallet(int walletId) override;

    void addRemovedCallback(std::function<void(int)> cb);
    void addUpdatedCallback(std::function<void(const Address&)> cb);
    void clearCallbacks();

private:
    Storage* storage_;

    std::mutex cbMutex_;
    std::vector<std::function<void()>> insertedCbs_;
    std::vector<std::function<void(const Address&)>> updatedCbs_;
    std::vector<std::function<void(int)>> removedCbs_;
};

class IDatabsae : public QObject {
    Q_OBJECT

public:
    virtual ~IDatabsae() = default;
    virtual IWalletGroupRepo* walletGroupRepo() = 0;
    virtual IWalletRepo* walletRepo() = 0;
    virtual IAddressRepo* addressRepo() = 0;

    template <typename Func>
    auto transaction(Func&& func) -> decltype(func())
    {
        return storage()->transaction(std::forward<Func>(func));
    }

    virtual Storage* storage() = 0;

    template <typename Func>
    auto safeTransaction(Func&& func) -> std::optional<decltype(func())>
    {
        try {
            return storage()->transaction([&] {
                try {
                    return func();
                } catch (const std::exception& e) {
                    qWarning()
                        << "Transaction failed, rolling back:" << e.what();
                    return false; // Return false will cause rollback
                }
            });
        } catch (const std::exception& e) {
            qCritical() << "Transaction failed:" << e.what();
            return std::nullopt;
        }
    }

Q_SIGNALS:
    void databaseError(DatabaseErrorType errorType, const QString& message);
    void databaseCorrupted();
    void migrationSuccessful(int newVersion);
    void backupSucceeded(const QString& backupFilePath);
    void backupFailed(const QString& errorMessage);
    void requestBackup(const QString& backupFilePath);
};

class Database : public IDatabsae {
    Q_OBJECT

public:
    Database(const QString& dataPath,
        const std::shared_ptr<const DatabaseConfig>& config);
    ~Database();

    IWalletGroupRepo* walletGroupRepo() override;
    IWalletRepo* walletRepo() override;
    IAddressRepo* addressRepo() override;

    Storage* storage() override;

    void reset();

    void startPeriodicBackups(int intervalHours = -1);

private Q_SLOTS:
    void checkScheduledBackup();

    void handleBackupCompleted(const QString& backupFilePath);

    void handleBackupFailed(const QString& errorMessage);

private:
    void performScheduledBackup();
    void cleanupOldBackups(const QString& backupDir, int maxCount);
    void handleError(DatabaseErrorType type, const std::string& message);

private:
    DatabaseContext context_;
    Storage* storage_;
    std::unique_ptr<IWalletGroupRepo> walletGroupRepo_;
    std::unique_ptr<IWalletRepo> walletRepo_;
    std::unique_ptr<IAddressRepo> addressRepo_;

    std::shared_ptr<const DatabaseConfig> config_;

    QTimer* backupTimer_ = nullptr;
    QDateTime lastBackupTime_;
    QThread* backupThread_ = nullptr;
    DatabaseBackupWorker* backupWorker_ = nullptr;
};
}
#endif // DATABASE_H
