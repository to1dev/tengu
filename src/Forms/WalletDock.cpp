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

#include "WalletDock.h"
#include "ui_WalletDock.h"

#include <spdlog/spdlog.h>

#include "Utils/Helpers.hpp"

using namespace Daitengu::Utils;

WalletDock::WalletDock(
    QWidget* parent, const std::shared_ptr<const GlobalManager>& globalManager)
    : QWidget(parent)
    , ui(new Ui::WalletDock)
    , globalManager_(globalManager)
{
    ui->setupUi(this);

    setWindowTitle(DEFAULT_TITLE);

    QVBoxLayout* layout = new QVBoxLayout(ui->frameContent);

    walletPanel_ = new WalletPanel(this);

    layout->addWidget(walletPanel_);
    // ui->frameContent->setLayout(layout);

    walletPanel_->userCard()->setRecord(
        globalManager_->settingManager()->record());

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->init(Frameless::Mode::PANEL);

    globalManager_->windowManager()->addWindow(
        WindowManager::WindowShape::RIGHT_PANEL, this);
    connect(frameless_.get(), &Frameless::onMax, this, [this]() {
        globalManager_->windowManager()->reset(
            this, 1, WindowManager::WindowShape::RIGHT_PANEL);
    });

    connect(globalManager_->settingManager()->database()->walletRepo(),
        WalletRepo::removed, [this](int id) {
            walletPanel_->userCard()->reset(id, -1);
            changeAddress();
        });

    connect(globalManager_->settingManager()->database()->walletRepo(),
        WalletRepo::updated, [this](const std::optional<Wallet>& wallet) {
            walletPanel_->userCard()->update(wallet, std::nullopt);
        });

    connect(globalManager_->settingManager()->database()->addressRepo(),
        AddressRepo::removed, [this](int id) {
            walletPanel_->userCard()->reset(-1, id);
            changeAddress();
        });

    connect(globalManager_->settingManager()->database()->addressRepo(),
        AddressRepo::updated, [this](const std::optional<Address>& address) {
            walletPanel_->userCard()->update(std::nullopt, address);
            changeAddress();
        });

    connect(walletPanel_->userCard(), &UserCard::onSelect, this,
        &WalletDock::select);

    monitor_ = new Monitor(this);
    monitor_->setRefreshInterval(30000);
    connect(monitor_, &Monitor::balanceUpdated, this,
        &WalletDock::onBalanceUpdated);
}

WalletDock::~WalletDock()
{
    globalManager_->settingManager()->setRecord(
        walletPanel_->userCard()->record());

    delete ui;
}

void WalletDock::showEvent(QShowEvent* event)
{
    globalManager_->windowManager()->reset(
        this, 1, WindowManager::WindowShape::RIGHT_PANEL);

    changeAddress();

    QWidget::showEvent(event);
}

void WalletDock::onBalanceUpdated(const Monitor::BalanceResult& result)
{
    std::string_view suffix
        = TickerSuffixes[static_cast<int>(result.chain) + 1];
    if (result.success) {
        walletPanel_->valueCard()->setValue(QString("%1 %2")
                .arg(QFormat::formatPrice(result.balance))
                .arg(suffix.data()));
    } else {
        walletPanel_->valueCard()->setValue(
            QString("%1 %2").arg(QFormat::formatPrice(0.0)).arg(suffix.data()));
    }
}

void WalletDock::changeAddress()
{
    auto address = walletPanel_->userCard()->record_ref().second.address;
    auto chainType = walletPanel_->userCard()->record_ref().first.chainType;
    monitor_->setAddress(
        static_cast<ChainType>(chainType), QString::fromStdString(address));
}

void WalletDock::select()
{
    WalletSelectorForm wsf(this, globalManager_);
    wsf.setRecord(walletPanel_->userCard()->record());
    int ret = wsf.exec();
    if (ret) {
        walletPanel_->userCard()->setRecord(wsf.record());
        // globalManager_->settingManager()->setRecord(wsf.record());
        changeAddress();
    }
}
