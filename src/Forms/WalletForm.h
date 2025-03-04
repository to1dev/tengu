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

#ifndef WALLETFORM_H
#define WALLETFORM_H

#include <QDialog>

#include "Consts.h"

#include "Managers/GlobalManager.h"
#include "Managers/WindowManager.h"

#include "Components/WalletListWidget.h"

#include "UI/Frameless.h"

#include "Forms/NewWalletForm.h"
#include "Forms/UpdateWalletForm.h"

using namespace Daitengu::Components;
using namespace Daitengu::Core;
using namespace Daitengu::UI;

namespace Ui {
class WalletForm;
}

class WalletForm : public QDialog {
    Q_OBJECT

    const QString DEFAULT_TITLE = QObject::tr("Wallets");

public:
    explicit WalletForm(QWidget* parent = nullptr,
        const std::shared_ptr<const GlobalManager>& globalManager = nullptr);
    ~WalletForm();

private Q_SLOTS:
    void ok();

private:
    void newWallet();
    void editWallet();
    void delWallet();

private:
    Ui::WalletForm* ui;

    std::shared_ptr<const GlobalManager> globalManager_;
    std::unique_ptr<Frameless> frameless_;

    WalletListWidget* walletList_;
};

#endif // WALLETFORM_H
