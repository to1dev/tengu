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

#include <QWidget>

#include "Managers/GlobalManager.h"
#include "Managers/WindowManager.h"

#include "UI/Frameless.h"

using namespace Daitengu::Core;
using namespace Daitengu::UI;

namespace Ui {
class WalletDock;
}

class WalletDock : public QWidget {
    Q_OBJECT

    const QString DEFAULT_TITLE = QObject::tr("My Wallet");

public:
    explicit WalletDock(QWidget* parent = nullptr,
        const std::shared_ptr<const GlobalManager>& globalManager = nullptr);
    ~WalletDock();

private:
    Ui::WalletDock* ui;

    std::shared_ptr<const GlobalManager> globalManager_;
    std::unique_ptr<Frameless> frameless_;
};
