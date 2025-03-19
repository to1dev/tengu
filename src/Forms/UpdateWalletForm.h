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

#include <algorithm>
#include <ranges>
#include <vector>

#include <QDialog>
#include <QVBoxLayout>

#include "Consts.h"

#include "Managers/GlobalManager.h"
#include "Managers/WindowManager.h"

#include "UI/Frameless.h"

#include "Utils/Encryption.h"

#include "Components/AddressListWidget.h"
#include "Components/ComboBoxEx.h"
#include "Components/LineEditEx.h"

#include "Databases/Database.h"

#include "Forms/MessageForm.h"
#include "Forms/NewAddressForm.h"

using namespace Daitengu::Components;
using namespace Daitengu::Core;
using namespace Daitengu::Databases;
using namespace Daitengu::UI;
using namespace Daitengu::Utils;

namespace Ui {
class UpdateWalletForm;
}

inline const QString CONFIRM_FIRST_WALLET_DELETE
    = QObject::tr("无法删除此地址！<p>请注意：保留地址无法删除！</p>");
inline const QString CONFIRM_ADDRESS_DELETE = QObject::tr(
    "是否确定删除<font "
    "color='orange'>[%1]</font>这个地址！<p>本操作不可逆！请务必谨慎！</p>");

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

class UpdateWalletForm : public QDialog {
    Q_OBJECT

    const QString DEFAULT_TITLE = QObject::tr("Edit Wallet");

public:
    struct UpdateWallet {
        int id = 0;
    };

    explicit UpdateWalletForm(const UpdateWallet& wallet,
        QWidget* parent = nullptr,
        const std::shared_ptr<const GlobalManager>& globalManager = nullptr);
    ~UpdateWalletForm();

    std::shared_ptr<Wallet> walletRecord() const;

private Q_SLOTS:
    void ok();
    void newAddress();
    void editAddress();
    void delAddress(const QModelIndex& index);

private:
    Ui::UpdateWalletForm* ui;

    std::shared_ptr<const GlobalManager> globalManager_;
    std::unique_ptr<Frameless> frameless_;

    AddressListWidget* addressList_;
    LineEditEx* editName_;
    ComboBoxEx* comboChain_;

    AddressManager addressManager_;

    UpdateWallet wallet_ {};

    std::shared_ptr<Wallet> walletRecord_;
};
