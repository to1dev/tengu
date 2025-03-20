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

#include "WalletSelectorForm.h"
#include "ui_WalletSelectorForm.h"

WalletSelectorForm::WalletSelectorForm(
    QWidget* parent, const std::shared_ptr<const GlobalManager>& globalManager)
    : QDialog(parent)
    , ui(new Ui::WalletSelectorForm)
    , globalManager_(globalManager)
    , addressRecord_(std::make_shared<Address>())
{
    ui->setupUi(this);

    setWindowTitle(DEFAULT_TITLE);

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->init(Frameless::Mode::DIALOG);

    QHBoxLayout* layout = new QHBoxLayout(ui->groupBox);
    layout->setContentsMargins(DEFAULT_GROUP_MARGINS);
    layout->setSpacing(20);

    walletView_ = new WalletListView(this);
    walletView_->load(
        globalManager_->settingManager()->database()->walletRepo()->getByGroup(
            static_cast<int>(WalletGroupType::User)));

    addressView_ = new AddressListView(this);

    layout->addWidget(walletView_);
    layout->addWidget(addressView_);
    ui->groupBox->setLayout(layout);

    globalManager_->windowManager()->reset(this, 0.6);
    connect(frameless_.get(), &Frameless::onMax, this,
        [this]() { globalManager_->windowManager()->reset(this, 0.6); });

    connect(ui->ButtonOK, &QPushButton::clicked, this, &WalletSelectorForm::ok);
    connect(ui->ButtonCancel, &QPushButton::clicked, this,
        &WalletSelectorForm::reject);
    connect(walletView_->selectionModel(), &QItemSelectionModel::currentChanged,
        this, &WalletSelectorForm::currentItemChanged);
}

WalletSelectorForm::~WalletSelectorForm()
{
    delete ui;
}

std::shared_ptr<Address> WalletSelectorForm::addressRecord() const
{
    return addressRecord_;
}

void WalletSelectorForm::ok()
{
}

void WalletSelectorForm::currentItemChanged(
    const QModelIndex& current, const QModelIndex& previous)
{
    if (current.isValid()) {
        int id = walletView_->model()
                     ->data(current,
                         static_cast<int>(WalletListModel::ItemData::Id))
                     .toInt();
        addressView_->load(globalManager_->settingManager()
                ->database()
                ->addressRepo()
                ->getAllByWallet(id));
    }
}
