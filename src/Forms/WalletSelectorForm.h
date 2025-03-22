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

#include <QDebug>
#include <QDialog>

#include "Consts.h"

#include "Managers/GlobalManager.h"
#include "Managers/WindowManager.h"

#include "UI/Frameless.h"

#include "Components/AddressListWidget.h"
#include "Components/WalletListWidget.h"

#include "Databases/Database.h"

#include "Forms/MessageForm.h"

using namespace Daitengu::Components;
using namespace Daitengu::Core;
using namespace Daitengu::Databases;
using namespace Daitengu::UI;

namespace Ui {
class WalletSelectorForm;
}

inline const QString NO_ADDRESS_SELECTED = QObject::tr("请选取一个有效地址！");

class WalletSelectorForm : public QDialog {
    Q_OBJECT

    const QString DEFAULT_TITLE = QObject::tr("Select Address");

public:
    explicit WalletSelectorForm(QWidget* parent = nullptr,
        const std::shared_ptr<const GlobalManager>& globalManager = nullptr);

    ~WalletSelectorForm();

    Record record() const;

private Q_SLOTS:
    void ok();
    void currentItemChanged(
        const QModelIndex& current, const QModelIndex& previous);

private:
    Ui::WalletSelectorForm* ui;

    std::shared_ptr<const GlobalManager> globalManager_;
    std::unique_ptr<Frameless> frameless_;

    WalletListView* walletView_;
    AddressListView* addressView_;

    Record record_;
};
