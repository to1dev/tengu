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
    if (config_->autoSyncSchema) {
        storage_->sync_schema();
    }

    storage_->pragma.journal_mode(config->journalMode);

    int currentVersion = getVersion(storage_.get());

    if (currentVersion < 1) {
        storage_->replace(Migration { 1, 1 });
        currentVersion = 1;
    }

#ifdef enable_migration
    if (currentVersion < 2) {
        // Migration
    }
#endif
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
    Q_EMIT inserted();
    return storage_->insert(wallet);
}

void WalletRepo::update(const Wallet& wallet)
{
    storage_->update(wallet);
    Q_EMIT updated(wallet);
}

void WalletRepo::remove(int id)
{
    storage_->remove_all<Address>(where(c(&Address::walletId) == id));
    storage_->remove<Wallet>(id);
    Q_EMIT removed(id);
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

int AddressRepo::insert(const Address& address)
{
    Q_EMIT inserted();
    return storage_->insert(address);
}

void AddressRepo::update(const Address& address)
{
    storage_->update(address);
    Q_EMIT updated(address);
}

void AddressRepo::remove(int id)
{
    storage_->remove<Address>(id);
    Q_EMIT removed(id);
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

Database::Database(const QString& dataPath,
    const std::shared_ptr<const DatabaseConfig>& config)
    : context_(dataPath, config)
    , config_(config)
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
    storage_->remove_all<Subscription>();
    storage_->remove_all<Order>();
    storage_->remove_all<addressTag>();
    storage_->remove_all<Tag>();
    storage_->remove_all<Address>();
    storage_->remove_all<Wallet>();
    storage_->remove_all<Network>();
    storage_->remove_all<WalletGroup>();
    storage_->vacuum();
}

}
