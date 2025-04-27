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

inline const QString STR_MENU_NETWORK = QObject::tr("当前网络");
inline const QString STR_MENU_MAINNET = QObject::tr("主网");
inline const QString STR_MENU_DEVNET = QObject::tr("开发网");
inline const QString STR_MENU_TESTNET = QObject::tr("测试网");
inline const QString STR_MENU_REFRESH = QObject::tr("刷新余额");
inline const QString STR_MENU_INFOR = QObject::tr("地址信息");

WalletDock::WalletDock(
    QWidget* parent, const std::shared_ptr<const GlobalManager>& globalManager)
    : QWidget(parent)
    , ui(new Ui::WalletDock)
    , globalManager_(globalManager)
{
    ui->setupUi(this);

    setWindowTitle(DEFAULT_TITLE);

    initPopup();
    updateActions();

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
    connect(
        walletPanel_->userCard(), &UserCard::onPopup, [this](const QPoint& pt) {
            QPoint p2 = QPoint(pt.x() - popup_->width(), pt.y());
            popup_->exec(p2);
        });

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

void WalletDock::initPopup()
{
    popup_ = new QMenu(this);
    netMenu_ = popup_->addMenu(STR_MENU_NETWORK);

    actions_ = new QActionGroup(this);
    actions_->setExclusive(true);

    popup_->addAction(STR_MENU_REFRESH, [this]() { monitor_->refresh(); });
    popup_->addSeparator();
    popup_->addAction(STR_MENU_INFOR, []() { qInfo() << "info"; });

    popup_->ensurePolished();
    popup_->adjustSize();

    // popup_->setWindowOpacity(0.95);
}

void WalletDock::updateActions()
{
    netMenu_->clear();
    for (QAction* action : actions_->actions()) {
        actions_->removeAction(action);
        delete action;
    }

    QAction* action1 = netMenu_->addAction(STR_MENU_MAINNET);
    action1->setCheckable(true);
    actions_->addAction(action1);
    action1->setChecked(true);

    QAction* action2 = netMenu_->addAction(STR_MENU_DEVNET);
    action2->setCheckable(true);
    actions_->addAction(action2);

    QAction* action3 = netMenu_->addAction(STR_MENU_TESTNET);
    action3->setCheckable(true);
    actions_->addAction(action3);

    /*bool first = true;
    for (const QString& network : networks) {
        QAction* action = netMenu_->addAction(network);
        action->setCheckable(true);
        actions_->addAction(action);

        if (first && defaultNetwork.isEmpty() || network == defaultNetwork) {
            action->setChecked(true);
            first = false;
        }
    }*/

    netMenu_->ensurePolished();
    netMenu_->adjustSize();
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
