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

#ifndef TENGU_H
#define TENGU_H

#include <QHBoxLayout>
#include <QMainWindow>
#include <QTabBar>

#include "Consts.h"
#include "Globals.h"

#include "Managers/GlobalManager.h"
#include "Managers/WindowManager.h"

#include "UI/Frameless.h"

#include "Forms/MessageForm.h"
#include "Forms/WalletDock.h"
#include "Forms/WalletForm.h"

using namespace Daitengu::Components;
using namespace Daitengu::Core;
using namespace Daitengu::UI;

QT_BEGIN_NAMESPACE

namespace Ui {
class Tengu;
}

QT_END_NAMESPACE

class Tengu : public QMainWindow {
    Q_OBJECT

    const QString DEFAULT_TITLE = QObject::tr("Tengu");

public:
    Tengu(const std::shared_ptr<const GlobalManager>& globalManager,
        QWidget* parent = nullptr);
    ~Tengu();

    void setWalletDock(const std::shared_ptr<WalletDock>& walletDock);

private Q_SLOTS:
    void showWalletDock();

    void reboot();
    void about();
    void wallet();

private:
    Ui::Tengu* ui;

    std::shared_ptr<WalletDock> walletDock_;

    std::shared_ptr<const GlobalManager> globalManager_;
    std::unique_ptr<Frameless> frameless_;
};
#endif // TENGU_H
