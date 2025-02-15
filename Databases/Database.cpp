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

Database::Database(const QString& dataPath)
    : context_(dataPath)
{
    storage_ = context_.storage();
    walletGroupRepo_ = std::make_unique<WalletGroupRepo>(storage_);
}

IWalletGroupRepo* Database::walletGroupRepo()
{
    return walletGroupRepo_.get();
}

Storage* Database::storage()
{
    return storage_;
}

void Database::reset()
{
    storage_->remove_all<WalletGroup>();
    storage_->vacuum();
}

}
