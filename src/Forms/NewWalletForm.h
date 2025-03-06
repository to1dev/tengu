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

#ifndef NEWWALLETFORM_H
#define NEWWALLETFORM_H

#include <QClipboard>
#include <QDialog>

#include "Consts.h"

#include "Managers/GlobalManager.h"
#include "Managers/WindowManager.h"

#include "UI/Frameless.h"

#include "Utils/Encryption.h"
#include "Utils/NameGenerator.h"

#include "Components/ComboBoxEx.h"
#include "Components/LineEditEx.h"
#include "Components/MnemonicView.h"

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
class NewWalletForm;
}

class NewWalletForm : public QDialog {
    Q_OBJECT

    const QString DEFAULT_TITLE = QObject::tr("New Wallet");

public:
    explicit NewWalletForm(QWidget* parent = nullptr,
        const std::shared_ptr<const GlobalManager>& globalManager = nullptr);
    ~NewWalletForm();

    std::shared_ptr<Wallet> walletRecord() const;

private:
    void refresh();

private Q_SLOTS:
    void ok();

private:
    Ui::NewWalletForm* ui;

    std::shared_ptr<const GlobalManager> globalManager_;
    std::unique_ptr<Frameless> frameless_;

    MnemonicView* view_;
    LineEditEx* editName_;
    ComboBoxEx* comboChain_;

    std::shared_ptr<Wallet> walletRecord_;
};

#endif // NEWWALLETFORM_H
