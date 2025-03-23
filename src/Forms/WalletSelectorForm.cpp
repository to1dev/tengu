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
    // ui->groupBox->setLayout(layout);

    globalManager_->windowManager()->reset(this, 0.7);
    connect(frameless_.get(), &Frameless::onMax, this,
        [this]() { globalManager_->windowManager()->reset(this, 0.7); });

    connect(ui->ButtonOK, &QPushButton::clicked, this, &WalletSelectorForm::ok);
    connect(ui->ButtonCancel, &QPushButton::clicked, this,
        &WalletSelectorForm::reject);

    connect(walletView_->selectionModel(), &QItemSelectionModel::currentChanged,
        this, &WalletSelectorForm::currentItemChanged);
    connect(addressView_, &QListView::doubleClicked,
        [this](const QModelIndex&) { ok(); });
}

WalletSelectorForm::~WalletSelectorForm()
{
    delete ui;
}

void WalletSelectorForm::setRecord(Record&& record)
{
    record_ = std::move(record);

    const int walletId = record_.first.id;
    if (walletId > 0) {
        const WalletListModel* model = walletView_->model();
        for (int i = 0; i < model->rowCount(); ++i) {
            int id = model->data(model->index(i), WalletListModel::ItemData::Id)
                         .toInt();
            if (id == walletId) {
                QModelIndex index = model->index(i);
                walletView_->setCurrentIndex(index);
                break;
            }
        }
    }
}

const Record& WalletSelectorForm::record_ref() const
{
    return record_;
}

Record WalletSelectorForm::record() const
{
    return record_;
}

void WalletSelectorForm::ok()
{
    QModelIndex indexWallet = walletView_->currentIndex();
    QModelIndex indexAddress = addressView_->currentIndex();
    if (indexWallet.isValid() && indexAddress.isValid()) {
        const WalletListModel* modelWallet = walletView_->model();
        const AddressListModel* modelAddress = addressView_->model();

        {
            record_.first.id
                = modelWallet->data(indexWallet, WalletListModel::ItemData::Id)
                      .toInt();
            record_.first.type
                = modelWallet->data(
                                 indexWallet, WalletListModel::ItemData::Type)
                      .toInt();
            record_.first.groupType
                = modelWallet
                      ->data(indexWallet, WalletListModel::ItemData::GroupType)
                      .toInt();
            record_.first.chainType
                = modelWallet
                      ->data(indexWallet, WalletListModel::ItemData::ChainType)
                      .toInt();
            record_.first.networkType
                = modelWallet
                      ->data(
                          indexWallet, WalletListModel::ItemData::NetworkType)
                      .toInt();
            record_.first.hash
                = modelWallet->data(
                                 indexWallet, WalletListModel::ItemData::Hash)
                      .toString()
                      .toStdString();
            record_.first.name
                = modelWallet->data(
                                 indexWallet, WalletListModel::ItemData::Name)
                      .toString()
                      .toStdString();
            record_.first.mnemonic
                = modelWallet
                      ->data(indexWallet, WalletListModel::ItemData::Mnemonic)
                      .toString()
                      .toStdString();
        }

        {
            record_.second.id
                = modelAddress
                      ->data(indexAddress,
                          static_cast<int>(AddressListModel::ItemData::Id))
                      .toInt();
            record_.second.type
                = modelAddress
                      ->data(indexAddress,
                          static_cast<int>(AddressListModel::ItemData::Type))
                      .toInt();
            record_.second.walletId
                = modelAddress
                      ->data(indexAddress,
                          static_cast<int>(
                              AddressListModel::ItemData::WalletId))
                      .toInt();
            record_.second.index
                = modelAddress
                      ->data(indexAddress,
                          static_cast<int>(AddressListModel::ItemData::Index))
                      .toInt();
            record_.second.hash
                = modelAddress
                      ->data(indexAddress,
                          static_cast<int>(AddressListModel::ItemData::Hash))
                      .toString()
                      .toStdString();
            record_.second.name
                = modelAddress
                      ->data(indexAddress,
                          static_cast<int>(AddressListModel::ItemData::Name))
                      .toString()
                      .toStdString();
            record_.second.address
                = modelAddress
                      ->data(indexAddress,
                          static_cast<int>(AddressListModel::ItemData::Address))
                      .toString()
                      .toStdString();
            record_.second.derivationPath
                = modelAddress
                      ->data(indexAddress,
                          static_cast<int>(
                              AddressListModel::ItemData::DerivationPath))
                      .toString()
                      .toStdString();
            record_.second.privateKey
                = modelAddress
                      ->data(indexAddress,
                          static_cast<int>(
                              AddressListModel::ItemData::PrivateKey))
                      .toString()
                      .toStdString();
        }

        accept();
    } else {
        MessageForm { nullptr, 5, NO_ADDRESS_SELECTED }.exec();
    }
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

        const int addressId = record_.second.id;
        if (addressId > 0) {
            const AddressListModel* model = addressView_->model();
            for (int i = 0; i < model->rowCount(); ++i) {
                int id
                    = model
                          ->data(model->index(i),
                              static_cast<int>(AddressListModel::ItemData::Id))
                          .toInt();
                if (id == addressId) {
                    QModelIndex index = model->index(i);
                    addressView_->setCurrentIndex(index);
                    break;
                }
            }
        }
    }
}
