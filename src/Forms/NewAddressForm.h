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

#include <QDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>

#include "Managers/GlobalManager.h"
#include "Managers/WindowManager.h"

#include "UI/Frameless.h"

#include "Utils/Encryption.h"

#include "Components/LineEditEx.h"
#include "Components/PlainTextEditEx.h"

#include "Wallets/Core/BaseMnemonic.h"
#include "Wallets/Core/BitcoinWallet.h"
#include "Wallets/Core/EthereumWallet.h"
#include "Wallets/Core/SolanaWallet.h"

#include "Databases/Database.h"

#include "Forms/MessageForm.h"

using namespace Daitengu::Components;
using namespace Daitengu::Core;
using namespace Daitengu::Databases;
using namespace Daitengu::UI;
using namespace Daitengu::Utils;
using namespace Daitengu::Wallets;

namespace Ui {
class NewAddressForm;
}

namespace User {

class AddressManager {
public:
    void addAddress(const Address& addr)
    {
        addresses_.push_back(addr);
    }

    void removeAddress(int index)
    {
        auto it = std::remove_if(addresses_.begin(), addresses_.end(),
            [index](const Address& addr) { return addr.index == index; });
        addresses_.erase(it, addresses_.end());
    }

    const std::vector<Address>& getAddresses() const
    {
        return addresses_;
    }

    int nextAvailableIndex() const
    {
        std::vector<int> indices;
        indices.reserve(addresses_.size());
        for (const auto& addr : addresses_) {
            indices.push_back(addr.index);
        }
        std::ranges::sort(indices);
        int freeIndex = 0;
        for (int idx : indices) {
            if (idx == freeIndex)
                ++freeIndex;
            else if (idx > freeIndex)
                break;
        }
        return freeIndex;
    }

private:
    std::vector<Address> addresses_;
};
}

class NewAddressForm : public QDialog {
    Q_OBJECT

    const QString DEFAULT_TITLE_NEW = QObject::tr("New Address");
    const QString DEFAULT_TITLE_EDIT = QObject::tr("Edit Address");

public:
    enum class Op {
        NEW,
        EDIT,
    };

    struct NewAddress {
        Op op = Op::NEW;
        int id = -1;
        int walletId = -1;
        int groupType = -1;
        int chainType = -1;
        int index = 0;
        std::string mnemonic = "";
    };

    explicit NewAddressForm(const NewAddress& address,
        QWidget* parent = nullptr,
        const std::shared_ptr<const GlobalManager>& globalManager = nullptr);

    ~NewAddressForm();

    std::shared_ptr<Address> addressRecord() const;

private Q_SLOTS:
    void ok();

private:
    Ui::NewAddressForm* ui;

    std::shared_ptr<const GlobalManager> globalManager_;
    std::unique_ptr<Frameless> frameless_;

    LineEditEx* editName_;
    PlainTextEditEx* text_;

    NewAddress address_ {};

    std::shared_ptr<Address> addressRecord_;
};
