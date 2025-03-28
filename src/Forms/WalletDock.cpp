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

WalletDock::WalletDock(
    QWidget* parent, const std::shared_ptr<const GlobalManager>& globalManager)
    : QWidget(parent)
    , ui(new Ui::WalletDock)
    , globalManager_(globalManager)
{
    ui->setupUi(this);

    setWindowTitle(DEFAULT_TITLE);

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->init(Frameless::Mode::PANEL);

    QVBoxLayout* layout = new QVBoxLayout(ui->frameContent);

    walletPanel_ = new WalletPanel(this);

    layout->addWidget(walletPanel_);
    // ui->frameContent->setLayout(layout);

    walletPanel_->userCard()->setRecord(
        globalManager_->settingManager()->record());

    globalManager_->windowManager()->addWindow(
        WindowManager::WindowShape::RIGHT_PANEL, this);
    connect(frameless_.get(), &Frameless::onMax, this, [this]() {
        globalManager_->windowManager()->reset(
            this, 1, WindowManager::WindowShape::RIGHT_PANEL);
    });

    connect(globalManager_->settingManager()->database()->walletRepo(),
        WalletRepo::removed,
        [this](int id) { walletPanel_->userCard()->reset(id, -1); });

    connect(globalManager_->settingManager()->database()->walletRepo(),
        WalletRepo::updated, [this](const std::optional<Wallet>& wallet) {
            walletPanel_->userCard()->update(wallet, std::nullopt);
        });

    connect(globalManager_->settingManager()->database()->addressRepo(),
        AddressRepo::removed,
        [this](int id) { walletPanel_->userCard()->reset(-1, id); });

    connect(globalManager_->settingManager()->database()->addressRepo(),
        AddressRepo::updated, [this](const std::optional<Address>& address) {
            walletPanel_->userCard()->update(std::nullopt, address);
        });

    connect(walletPanel_->userCard(), &UserCard::doSelect, this,
        &WalletDock::select);
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

    QWidget::showEvent(event);
}

void WalletDock::select()
{
    WalletSelectorForm wsf(this, globalManager_);
    wsf.setRecord(walletPanel_->userCard()->record());
    int ret = wsf.exec();
    if (ret) {
        walletPanel_->userCard()->setRecord(wsf.record());
        // globalManager_->settingManager()->setRecord(wsf.record());
    }
}
