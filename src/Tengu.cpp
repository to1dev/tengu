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

#include "Tengu.h"
#include "ui_Tengu.h"

Tengu::Tengu(
    const std::shared_ptr<const GlobalManager>& globalManager, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::Tengu)
    , globalManager_(globalManager)
{
    ui->setupUi(this);
    setWindowTitle(DEFAULT_TITLE);

#ifdef RELEASE
    ui->ButtonNews->setEnabled(false);
    ui->ButtonChart->setEnabled(false);
    ui->ButtonTrade->setEnabled(false);
    ui->ButtonSwap->setEnabled(false);
    ui->ButtonBridge->setEnabled(false);
    ui->ButtonArbitrage->setEnabled(false);
    ui->ButtonDefi->setEnabled(false);
    ui->ButtonScript->setEnabled(false);
    ui->ButtonAI->setEnabled(false);
#endif

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setTopFrame(ui->topFrame);
    frameless_->setMainMenu(ui->menubar);
    frameless_->init();

    // For round corner
    /*ui->menu_File->setWindowFlags(
        ui->menu_File->windowFlags() | Qt::FramelessWindowHint);
    ui->menu_File->setAttribute(Qt::WA_TranslucentBackground);*/

    globalManager_->windowManager()->addWindow(
        WindowManager::WindowShape::TOPBAR, this);
    globalManager_->windowManager()->reset(
        this, 1, WindowManager::WindowShape::TOPBAR);

    connect(frameless_.get(), &Frameless::onMin, this, &Tengu::showMinimized);
    connect(frameless_.get(), &Frameless::onMax, this, [this]() {
        globalManager_->windowManager()->reset(
            this, 1, WindowManager::WindowShape::TOPBAR);
    });

    connect(ui->ButtonWallet, &QToolButton::clicked, this, &Tengu::wallet);
    connect(ui->actionOpen, &QAction::triggered, this, &Tengu::showWalletDock);
    connect(ui->actionReboot, &QAction::triggered, this, &Tengu::reboot);
}

Tengu::~Tengu()
{
    delete ui;
}

void Tengu::setWalletDock(const std::shared_ptr<WalletDock>& walletDock)
{
    walletDock_ = walletDock;
}

void Tengu::showWalletDock()
{
    if (auto wd = walletDock_.lock()) {
        wd->show();
    } else {
        spdlog::error("WalletDock is no longer available");
    }
}

void Tengu::reboot()
{
    QApplication::exit(Daitengu::Core::EXIT_CODE_REBOOT);
}

void Tengu::about()
{
    MessageForm mf { this, 100,
        "<strong>关于</strong> 这个程序！<p>Hello world!</p>", "About" };
    mf.exec();
}

void Tengu::wallet()
{
    WalletForm wf(this, globalManager_);
    int ret = wf.exec();
    if (ret) { }
}
