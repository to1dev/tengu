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

#include <QMenu>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QWidget>

#include "Managers/GlobalManager.h"
#include "Managers/WindowManager.h"

#include "UI/Frameless.h"

#include "Components/WalletPanel.h"

#include "Clients/Core/Hydra/Hydra.h"
#include "Clients/Core/Hydra/PriceDataSource.h"

#include "Wallets/Monitor/Monitor.h"

#include "Forms/WalletSelectorForm.h"

using namespace Daitengu::Clients::Hydra;
using namespace Daitengu::Core;
using namespace Daitengu::UI;
using namespace Daitengu::Wallets;

namespace Ui {
class WalletDock;
}

namespace {
inline constexpr std::array<std::string_view, 5> TickerSuffixes = {
    "",
    "BTC",
    "ETH",
    "SOL",
    "SUI",
};
}

class WalletDock : public QWidget {
    Q_OBJECT

    const QString DEFAULT_TITLE = QObject::tr("My Wallet");

public:
    explicit WalletDock(QWidget* parent = nullptr,
        const std::shared_ptr<const GlobalManager>& globalManager = nullptr);
    ~WalletDock();

protected:
    void showEvent(QShowEvent* event) override;

private Q_SLOTS:
    void select();
    void onBalanceUpdated(const Monitor::BalanceResult& result);

private:
    void initPopup();
    void updateActions();
    void changeAddress();

private:
    Ui::WalletDock* ui;

    std::shared_ptr<const GlobalManager> globalManager_;
    std::unique_ptr<Frameless> frameless_;

    QMenu* popup_;
    QMenu* netMenu_;
    QActionGroup* actions_;

    Monitor* monitor_;

    WalletPanel* walletPanel_;
};
