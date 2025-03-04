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
#include <optional>
#include <string>
#include <vector>

#include <QDir>
#include <QString>

#include "sqlite_orm/sqlite_orm.h"

#include "Wallets/Core/Errors.hpp"
#include "Wallets/Core/Types.h"

using namespace sqlite_orm;

using namespace Daitengu::Wallets;

namespace Daitengu::Databases {

inline constexpr char DB_NAME[] = "tengu";

enum class WalletGroupType {
    User = 0,
    Vault = 1,
};

struct WalletGroup {
    int id = 0;
    int type = 0;
    std::string name = "";
};

enum class WalletType {
    Original = 0,
    Mnemonic,
    Priv,
    Wif,
    Address,
};

struct Wallet {
    int id = 0;
    int type = 0;
    int groupId = 0;
    int chainType = 0;
    int networkType = 0;
    std::string hash = "";
    std::string name = "";
    std::string nameHash = "";
    std::string mnemonic = "";
    std::string mnemonicHash = "";
    std::string passphrase = "";
    std::string masterPrivateKey = "";
    std::string extendedPublicKey = "";
};

struct Address {
    int id = 0;
    int type = 0;
    int walletId = 0;
    std::string hash = "";
    std::string name = "";
    std::string nameHash = "";
    std::string address = "";
    std::string addressHash = "";
    std::string derivationPath = "";
    std::string privateKey = "";
    std::string publicKey = "";
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

enum class DBErrorType {
    none = 0,
    haveName,
    haveMnemonic,
};

inline auto initStorage(const QString& dataPath)
{
    QString filePath = QDir::toNativeSeparators(
        QString("%1/%2.db").arg(dataPath).arg(DB_NAME));

    return make_storage(filePath.toStdString(),
        make_table("groups",
            make_column("id", &WalletGroup::id, primary_key().autoincrement()),
            make_column("type", &WalletGroup::type),
            make_column("name", &WalletGroup::name)),

        make_table("wallets",
            make_column("id", &Wallet::id, primary_key().autoincrement()),
            make_column("type", &Wallet::type),
            make_column("groupId", &Wallet::groupId),
            make_column("chainType", &Wallet::chainType),
            make_column("networkType", &Wallet::networkType),
            make_column("hash", &Wallet::hash),
            make_column("name", &Wallet::name),
            make_column("nameHash", &Wallet::nameHash),
            make_column("mnemonic", &Wallet::mnemonic),
            make_column("mnemonicHash", &Wallet::mnemonicHash),
            make_column("passphrase", &Wallet::passphrase),
            make_column("masterPrivateKey", &Wallet::masterPrivateKey),
            make_column("extendedPublicKey", &Wallet::extendedPublicKey)),

        make_table("addresses",
            make_column("id", &Address::id, primary_key().autoincrement()),
            make_column("type", &Address::type),
            make_column("walletId", &Address::walletId),
            make_column("hash", &Address::hash),
            make_column("name", &Address::name),
            make_column("nameHash", &Address::nameHash),
            make_column("address", &Address::address),
            make_column("addressHash", &Address::addressHash),
            make_column("derivationPath", &Address::derivationPath),
            make_column("privateKey", &Address::privateKey),
            make_column("publicKey", &Address::publicKey)),

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

class DatabaseContext {
public:
    DatabaseContext(const QString& dataPath);

    Storage* storage();

private:
    std::unique_ptr<Storage> storage_;
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
    virtual std::vector<Wallet> getByGroup(int groupId) = 0;

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
    std::vector<Wallet> getByGroup(int groupId) override;

private:
    Storage* storage_;
};

class IAddressRepo {
public:
    virtual ~IAddressRepo() = default;
    virtual DBErrorType before(const Address& group, bool update = false) = 0;
    virtual int insert(const Address& address) = 0;
    virtual void update(const Address& address) = 0;
    virtual void remove(int id) = 0;
    virtual std::optional<Address> get(int id) = 0;
    virtual std::vector<Address> getAllByWallet(int walletId) = 0;
};

class AddressRepo : public QObject, public IAddressRepo {
    Q_OBJECT

public:
    AddressRepo(Storage* storage);
    DBErrorType before(const Address& address, bool update) override;
    int insert(const Address& address) override;
    void update(const Address& address) override;
    void remove(int id) override;
    std::optional<Address> get(int id) override;
    std::vector<Address> getAllByWallet(int walletId) override;

Q_SIGNALS:
    void inserted();
    void updated(const Address& address);
    void removed(int id);

private:
    Storage* storage_;
};

class IDatabsae {
public:
    virtual ~IDatabsae() = default;
    virtual IWalletGroupRepo* walletGroupRepo() = 0;
    virtual IWalletRepo* walletRepo() = 0;
    virtual IAddressRepo* addressRepo() = 0;

    template <typename Func> auto transaction(Func&& func) -> decltype(func())
    {
        return storage()->transaction(std::forward<Func>(func));
    }

    virtual Storage* storage() = 0;
};

class Database : public IDatabsae {
public:
    Database(const QString& dataPath);

    IWalletGroupRepo* walletGroupRepo() override;
    IWalletRepo* walletRepo() override;
    IAddressRepo* addressRepo() override;

    Storage* storage() override;

    void reset();

private:
    DatabaseContext context_;
    Storage* storage_;
    std::unique_ptr<IWalletGroupRepo> walletGroupRepo_;
    std::unique_ptr<IWalletRepo> walletRepo_;
    std::unique_ptr<IAddressRepo> addressRepo_;
};

}
#endif // DATABASE_H
