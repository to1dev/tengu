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

#include "WalletForm.h"
#include "ui_WalletForm.h"

WalletForm::WalletForm(
    QWidget* parent, const std::shared_ptr<const GlobalManager>& globalManager)
    : QDialog(parent)
    , ui(new Ui::WalletForm)
    , globalManager_(globalManager)
{
    ui->setupUi(this);

    setWindowTitle(DEFAULT_TITLE);

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->init(Frameless::Mode::DIALOG);

    QHBoxLayout* layoutPanel = new QHBoxLayout(ui->groupBoxWallets);
    layoutPanel->setContentsMargins(DEFAULT_GROUP_MARGINS);

    walletList_ = new WalletListWidget(this);

    QWidget* panelButtons = new QWidget(this);
    panelButtons->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    panelButtons->setContentsMargins(QMargins(0, 0, 0, 0));

    QVBoxLayout* layoutButtons = new QVBoxLayout(panelButtons);
    layoutButtons->setContentsMargins(QMargins(20, 0, 0, 0));
    layoutButtons->setSpacing(DEFAULT_SPACING);
    layoutButtons->addWidget(ui->ButtonNewWallet);
    layoutButtons->addWidget(ui->ButtonImportWallet);
    layoutButtons->addWidget(ui->ButtonEditWallet);
    layoutButtons->addWidget(ui->ButtonDeleteWallet);
    layoutButtons->addSpacing(16);
    layoutButtons->addWidget(ui->ButtonSmartWallet);
    layoutButtons->addStretch(1);
    panelButtons->setLayout(layoutButtons);

    layoutPanel->addWidget(walletList_);
    layoutPanel->addWidget(panelButtons);
    ui->groupBoxWallets->setLayout(layoutPanel);

    globalManager_->windowManager()->reset(this, 0.8);
    connect(frameless_.get(), &Frameless::onMax, this,
        [this]() { globalManager_->windowManager()->reset(this, 0.8); });

    connect(ui->ButtonOK, &QPushButton::clicked, this, &WalletForm::ok);
    connect(ui->ButtonNewWallet, &QPushButton::clicked, this,
        &WalletForm::newWallet);
    connect(ui->ButtonImportWallet, &QPushButton::clicked, this,
        &WalletForm::importWallet);
    connect(ui->ButtonEditWallet, &QPushButton::clicked, this,
        &WalletForm::editWallet);
    connect(walletList_, &WalletListWidget::itemDoubleClicked, this,
        &WalletForm::editWallet);
    connect(ui->ButtonDeleteWallet, &QPushButton::clicked, this,
        &WalletForm::delWallet);

    walletList_->load(
        globalManager_->settingManager()->database()->walletRepo()->getAll());
}

WalletForm::~WalletForm()
{
    delete ui;
}

void WalletForm::ok()
{
    accept();
}

void WalletForm::newWallet()
{
    NewWalletForm::NewWallet wallet;
    NewWalletForm nwf(wallet, this, globalManager_);
    int ret = nwf.exec();
    if (ret) {
        walletList_->add(*nwf.walletRecord());
    } else {
    }
}

void WalletForm::importWallet()
{
    ImportWalletForm iwf(this, globalManager_);
    int ret = iwf.exec();
    if (ret) {
        walletList_->add(*iwf.walletRecord());
    } else {
    }
}

void WalletForm::editWallet()
{
    if (auto* item = walletList_->currentItem(); item && item->isSelected()) {
        const auto id
            = item->data(static_cast<int>(WalletListWidget::ItemData::id))
                  .toInt();

        UpdateWalletForm::UpdateWallet wallet {
            .id = id,
        };
        UpdateWalletForm uwf(wallet, this, globalManager_);

        int ret = uwf.exec();
        if (ret) {
            walletList_->update(*uwf.walletRecord());
        } else {
        }
    }
}

void WalletForm::delWallet()
{
    /*QListWidgetItem* item = walletList_->currentItem();
    if (item && item->isSelected()) {
        int id = item->data(static_cast<int>(WalletListWidget::ItemData::id))
                     .toInt();
        MessageForm mf(this, -1, CONFIRM_WALLET_DELETE,
    CONFIRM_WALLET_DELETE_TITLE, MessageButton::Ok | MessageButton::Cancel); int
    ret = mf.exec(); if (ret) { }
    }*/

    if (auto* item = walletList_->currentItem(); item && item->isSelected()) {
        const auto id
            = item->data(static_cast<int>(WalletListWidget::ItemData::id))
                  .toInt();
        const auto name
            = item->data(static_cast<int>(WalletListWidget::ItemData::name))
                  .toString();
        MessageForm mf(this, 14, CONFIRM_WALLET_DELETE.arg(name),
            CONFIRM_WALLET_DELETE_TITLE, true,
            MessageButton::Ok | MessageButton::Cancel);
        if (mf.exec()) {
            try {
                globalManager_->settingManager()
                    ->database()
                    ->walletRepo()
                    ->remove(id);
                std::unique_ptr<QListWidgetItem> removedItem {
                    walletList_->takeItem(walletList_->row(item))
                };
                walletList_->clearSelection();
            } catch (const DatabaseException& e) {
                std::cerr << "Failed to remove wallet: " << e.what()
                          << std::endl;
            }
        }
    }
}
