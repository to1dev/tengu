#include "Database.h"

namespace Daitengu::Core {

DatabaseContext::DatabaseContext(const QString& dataPath)
    : storage_(std::make_unique<Storage>(initStorage(dataPath)))
{
    storage_->sync_schema();
}

Storage* DatabaseContext::storage()
{
    return storage_.get();
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

DBErrorType WalletRepo::before(const Wallet& group, bool update)
{
    return DBErrorType::none;
}

int WalletRepo::insert(const Wallet& wallet)
{
    return storage_->insert(wallet);
}

void WalletRepo::update(const Wallet& wallet)
{
    storage_->update(wallet);
}

void WalletRepo::remove(int id)
{
    storage_->remove_all<Address>(where(c(&Address::walletId) == id));
    storage_->remove<Wallet>(id);
}

std::optional<Wallet> WalletRepo::get(int id)
{
    if (auto wallet = storage_->get_pointer<Wallet>(id))
        return *wallet;
    return std::nullopt;
}

std::vector<Wallet> WalletRepo::getAll()
{
    return storage_->get_all<Wallet>();
}

std::vector<Wallet> WalletRepo::getByGroup(int groupId)
{
    return storage_->get_all<Wallet>(where(c(&Wallet::groupId) == groupId));
}

AddressRepo::AddressRepo(Storage* storage)
    : storage_(storage)
{
}

DBErrorType AddressRepo::before(const Address& group, bool update)
{
    return DBErrorType::none;
}

int AddressRepo::insert(const Address& address)
{
    return storage_->insert(address);
}

void AddressRepo::update(const Address& address)
{
    storage_->update(address);
}

void AddressRepo::remove(int id)
{
    storage_->remove<Address>(id);
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

Database::Database(const QString& dataPath)
    : context_(dataPath)
{
    storage_ = context_.storage();
    walletGroupRepo_ = std::make_unique<WalletGroupRepo>(storage_);
    walletRepo_ = std::make_unique<WalletRepo>(storage_);
    addressRepo_ = std::make_unique<AddressRepo>(storage_);
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
    storage_->remove_all<Address>();
    storage_->remove_all<Wallet>();
    storage_->remove_all<WalletGroup>();
    storage_->vacuum();
}

}
