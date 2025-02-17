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

using namespace sqlite_orm;

namespace Daitengu::Core {

inline constexpr char DB_NAME[] = "tengu";

enum class WalletGroupType {
    User = 0,
    Smart = 1,
};

struct WalletGroup {
    int id;
    std::string name;
    int type;
};

struct Wallet {
    int id;
    int type;
    int groupId;
    std::string hash;
    std::string name;
    std::string nameHash;
    std::string mnemonic;
    std::string mnemonicHash;
    std::string derivationPath;
};

struct Address {
    int id;
    int type;
    int walletId;
    int walletType;
    std::string hash;
    std::string name;
    std::string nameHash;
    std::string address;
    std::string addressHash;
    std::string derivationPath;
    std::string privKey;
    std::string pubKey;
    std::string wifKey;
};

struct Tag {
    int id;
    std::string name;
    std::string description;
    long long createTimestamp;
};

struct addressTag {
    int id;
    int addressId;
    int tagId;
    long long timestamp;
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
    int id;
    int walletId;
    std::string symbol;
    std::string side;
    double quantity;
    double price;
    double stopPrice;
    int orderType;
    int status;
    double executedQuantity;
    double avgExecutionPrice;
    long long createTimestamp;
    long long updateTimestamp;
    bool trailingStopEnabled;
    double trailingStopOffset;
    double currentTrailingStopPrice;
    bool trailingTakeProfitEnabled;
    double trailingTakeProfitOffset;
    double currentTrailingTakeProfitPrice;
};

struct Subscription {
    int id;
    int walletId;
    bool subscribed;
    bool followed;
    bool pinned;
    long long timestamp;
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
        make_table("wallet_groups",
            make_column("id", &WalletGroup::id, primary_key().autoincrement()),
            make_column("name", &WalletGroup::name),
            make_column("type", &WalletGroup::type)),

        make_table("wallets",
            make_column("id", &Wallet::id, primary_key().autoincrement()),
            make_column("type", &Wallet::type),
            make_column("groupId", &Wallet::groupId),
            make_column("hash", &Wallet::hash),
            make_column("name", &Wallet::name),
            make_column("nameHash", &Wallet::nameHash),
            make_column("mnemonic", &Wallet::mnemonic),
            make_column("mnemonicHash", &Wallet::mnemonicHash),
            make_column("derivationPath", &Wallet::derivationPath)),

        make_table("subscriptions",
            make_column("id", &Subscription::id, primary_key().autoincrement()),
            make_column("walletId", &Subscription::walletId),
            make_column("subscribed", &Subscription::subscribed),
            make_column("followed", &Subscription::followed),
            make_column("pinned", &Subscription::pinned),
            make_column("timestamp", &Subscription::timestamp)),

        make_table("addresses",
            make_column("id", &Address::id, primary_key().autoincrement()),
            make_column("type", &Address::type),
            make_column("walletId", &Address::walletId),
            make_column("walletType", &Address::walletType),
            make_column("hash", &Address::hash),
            make_column("name", &Address::name),
            make_column("nameHash", &Address::nameHash),
            make_column("address", &Address::address),
            make_column("addressHash", &Address::addressHash),
            make_column("derivationPath", &Address::derivationPath),
            make_column("privKey", &Address::privKey),
            make_column("pubKey", &Address::pubKey),
            make_column("wifKey", &Address::wifKey)),

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
            make_column("trailingStopEnabled", &Order::updateTimestamp),
            make_column("trailingStopOffset", &Order::updateTimestamp),
            make_column("currentTrailingStopPrice", &Order::updateTimestamp),
            make_column("trailingTakeProfitEnabled", &Order::updateTimestamp),
            make_column("trailingTakeProfitOffset", &Order::updateTimestamp),
            make_column(
                "currentTrailingTakeProfitPrice", &Order::updateTimestamp))

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
    virtual int insert(const WalletGroup& group) = 0;
    virtual void update(const WalletGroup& group) = 0;
    virtual void remove(int id) = 0;
    virtual std::optional<WalletGroup> get(int id) = 0;
    virtual std::vector<WalletGroup> getAll() = 0;
};

class WalletGroupRepo : public IWalletGroupRepo {
public:
    WalletGroupRepo(Storage* storage);
    int insert(const WalletGroup& group) override;
    void update(const WalletGroup& group) override;
    void remove(int id) override;
    std::optional<WalletGroup> get(int id) override;
    std::vector<WalletGroup> getAll() override;

private:
    Storage* storage_;
};

class IWalletRepo {
public:
    virtual ~IWalletRepo() = default;
    virtual int insert(const Wallet& wallet) = 0;
    virtual void update(const Wallet& wallet) = 0;
    virtual void remove(int id) = 0;
    virtual std::optional<Wallet> get(int id) = 0;
    virtual std::vector<Wallet> getAll() = 0;
    virtual std::vector<Wallet> getByGroup(int groupId) = 0;
};

class WalletRepo : public IWalletRepo {
public:
    WalletRepo(Storage* storage);
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
    virtual int insert(const Address& address) = 0;
    virtual void update(const Address& address) = 0;
    virtual void remove(int id) = 0;
    virtual std::optional<Address> get(int id) = 0;
    virtual std::vector<Address> getAllByWallet(int walletId) = 0;
};

class AddressRepo : public IAddressRepo {
public:
    AddressRepo(Storage* storage);
    int insert(const Address& address) override;
    void update(const Address& address) override;
    void remove(int id) override;
    std::optional<Address> get(int id) override;
    std::vector<Address> getAllByWallet(int walletId) override;

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
