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

#include "Consts.h"

#include "Managers/GlobalManager.h"
#include "Managers/WindowManager.h"

#include "UI/Frameless.h"

#include "Utils/Encryption.h"
#include "Utils/Helpers.hpp"
#include "Utils/NameGenerator.h"

#include "Components/AddressImport/CryptoAddressEdit.h"
#include "Components/ComboBoxEx.h"
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
class SmartWalletForm;
}

namespace {
inline const QString NO_VALID_ADDRESSES = QObject::tr("请输入有效的地址列表！");
}

class SmartWalletForm : public QDialog {
    Q_OBJECT

    const QString DEFAULT_TITLE = QObject::tr("Smart Wallet");

public:
    explicit SmartWalletForm(QWidget* parent = nullptr,
        const std::shared_ptr<const GlobalManager>& globalManager = nullptr);
    ~SmartWalletForm();

    std::shared_ptr<Wallet> walletRecord() const;

private Q_SLOTS:
    void ok();

private:
    Ui::SmartWalletForm* ui;

    std::shared_ptr<const GlobalManager> globalManager_;
    std::unique_ptr<Frameless> frameless_;

    LineEditEx* editName_;
    ComboBoxEx* comboChain_;
    CryptoAddressEdit* text_;
    PlainTextEditEx* desc_;

    std::shared_ptr<Wallet> walletRecord_;
};
