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

    QHBoxLayout* layoutPanel = new QHBoxLayout(ui->groupBoxWallets);
    layoutPanel->setContentsMargins(DEFAULT_GROUP_MARGINS);

    walletView_ = new WalletListView(this);
    walletView_->load(
        globalManager_->settingManager()->database()->walletRepo()->getAll());

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
    // panelButtons->setLayout(layoutButtons);

    layoutPanel->addWidget(walletView_);
    layoutPanel->addWidget(panelButtons);
    // ui->groupBoxWallets->setLayout(layoutPanel);

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->init(Frameless::Mode::DIALOG);

    globalManager_->windowManager()->reset(this, 0.8);
    connect(frameless_.get(), &Frameless::onMax, this,
        [this]() { globalManager_->windowManager()->reset(this, 0.8); });

    connect(walletView_, &WalletListView::doubleClicked, this,
        &WalletForm::editWallet);

    connect(ui->ButtonOK, &QPushButton::clicked, this, &WalletForm::ok);
    connect(ui->ButtonNewWallet, &QPushButton::clicked, this,
        &WalletForm::newWallet);
    connect(ui->ButtonImportWallet, &QPushButton::clicked, this,
        &WalletForm::importWallet);
    connect(ui->ButtonEditWallet, &QPushButton::clicked, this,
        &WalletForm::editWallet);
    connect(ui->ButtonDeleteWallet, &QPushButton::clicked, this,
        &WalletForm::delWallet);
    connect(ui->ButtonSmartWallet, &QPushButton::clicked, this,
        &WalletForm::smartWallet);
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
        walletView_->add(*nwf.walletRecord());
    } else {
    }
}

void WalletForm::importWallet()
{
    ImportWalletForm iwf(this, globalManager_);
    int ret = iwf.exec();
    if (ret) {
        walletView_->add(*iwf.walletRecord());
    } else {
    }
}

void WalletForm::editWallet()
{
    QModelIndex index = walletView_->currentIndex();
    if (!index.isValid())
        return;

    auto model = walletView_->model();

    int id = model->data(index, static_cast<int>(WalletListModel::ItemData::Id))
                 .toInt();

    int groupType
        = model
              ->data(
                  index, static_cast<int>(WalletListModel::ItemData::GroupType))
              .toInt();

    if (groupType == static_cast<int>(WalletGroupType::Smart)) {
        SmartWalletForm swf(this, globalManager_);
        int ret = swf.exec();
        if (ret) {
        } else {
        }
    } else {
        UpdateWalletForm::UpdateWallet wallet {
            .id = id,
        };
        UpdateWalletForm uwf(wallet, this, globalManager_);

        int ret = uwf.exec();
        if (ret) {
            walletView_->update(*uwf.walletRecord());
        } else {
        }
    }
}

void WalletForm::delWallet()
{
    QModelIndex index = walletView_->currentIndex();
    if (!index.isValid())
        return;

    auto model = walletView_->model();

    int id = model->data(index, static_cast<int>(WalletListModel::ItemData::Id))
                 .toInt();
    QString name
        = model->data(index, static_cast<int>(WalletListModel::ItemData::Name))
              .toString();
    int groupType
        = model
              ->data(
                  index, static_cast<int>(WalletListModel::ItemData::GroupType))
              .toInt();
    int type
        = model->data(index, static_cast<int>(WalletListModel::ItemData::Type))
              .toInt();

    bool doubleCheck = false;
    QString msg;
    if (groupType == static_cast<int>(WalletGroupType::User)) {
        msg = CONFIRM_WALLET_DELETE.arg(name);
        doubleCheck = true;
    } else {
        if (type == static_cast<int>(WalletType::Mnemonic)
            || type == static_cast<int>(WalletType::Priv)
            || type == static_cast<int>(WalletType::Wif)) {
            msg = CONFIRM_WALLET_DELETE.arg(name);
            doubleCheck = true;
        } else {
            msg = CONFIRM_IMPORT_DELETE.arg(name);
        }
    }

    MessageForm mf { this, 14, msg, CONFIRM_WALLET_DELETE_TITLE, doubleCheck,
        MessageButton::Ok | MessageButton::Cancel };
    if (mf.exec()) {
        try {
            globalManager_->settingManager()->database()->walletRepo()->remove(
                id);
            walletView_->remove(QList<int>() << index.row());
        } catch (const DatabaseException& e) {
            std::cerr << "Failed to remove wallet: " << e.what() << std::endl;
        }
    }
}

void WalletForm::smartWallet()
{
    SmartWalletForm swf(this, globalManager_);
    int ret = swf.exec();
    if (ret) {
    } else {
    }
}
