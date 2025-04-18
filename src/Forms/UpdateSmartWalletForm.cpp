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

#include "UpdateSmartWalletForm.h"
#include "ui_UpdateSmartWalletForm.h"

namespace {
inline const QString CONFIRM_FIRST_WALLET_DELETE
    = QObject::tr("无法删除此地址！<p>请注意：保留地址无法删除！</p>");
inline const QString CONFIRM_ADDRESS_DELETE
    = QObject::tr("是否确定删除<font "
                  "color='orange'>[%1]</"
                  "font>这个地址！<p>本操作有可能引起不便！请务必谨慎！</p>");
}

UpdateSmartWalletForm::UpdateSmartWalletForm(const UpdateSmartWallet& wallet,
    QWidget* parent, const std::shared_ptr<const GlobalManager>& globalManager)
    : QDialog(parent)
    , ui(new Ui::UpdateSmartWalletForm)
    , wallet_(wallet)
    , globalManager_(globalManager)
{
    ui->setupUi(this);

    setWindowTitle(DEFAULT_TITLE);

    QVBoxLayout* layout = new QVBoxLayout(ui->groupBox);
    layout->setContentsMargins(DEFAULT_GROUP_MARGINS);
    layout->setSpacing(DEFAULT_SPACING);

    addressView_ = new AddressListView(this, true);
    layout->addWidget(addressView_);

    QVBoxLayout* layoutOptions = new QVBoxLayout(ui->groupBoxOptions);
    layoutOptions->setContentsMargins(DEFAULT_GROUP_MARGINS);
    layoutOptions->setSpacing(DEFAULT_SPACING);

    QLabel* labelName = new QLabel(this);
    labelName->setText(STR_LABEL_NAME);

    editName_ = new LineEditEx(this);
    editName_->setMaxLength(DEFAULT_MAXLENGTH);
    editName_->setPlaceholderText(STR_LINEEDIT_WALLET_NAME_PLACEHOLDER);
    editName_->setCursorPosition(0);

    QLabel* labelChain = new QLabel(this);
    labelChain->setText(STR_LABEL_CHAIN);

    comboChain_ = new ComboBoxEx(this);
    comboChain_->setEnabled(false);

    int index = 0;
    for (const auto& chain : Chains) {
        comboChain_->addItem(QString::fromUtf8(
            chain.second.name.data(), chain.second.name.size()));
        bool enabled = chain.second.enabled;
        if (!enabled) {
            comboChain_->setItemEnabled(index, false);
        }
        index++;
    }

    QLabel* labelDesc = new QLabel(this);
    labelDesc->setText(STR_LABEL_DESC);

    desc_ = new PlainTextEditEx(this);
    desc_->setObjectName("plainDesc");

    layoutOptions->addWidget(labelName);
    layoutOptions->addWidget(editName_);
    layoutOptions->addWidget(labelChain);
    layoutOptions->addWidget(comboChain_);
    layoutOptions->addWidget(labelDesc);
    layoutOptions->addWidget(desc_, 1);

    ui->ButtonOK->setDefault(true);

    auto opt = globalManager_->settingManager()->database()->walletRepo()->get(
        wallet_.id);
    if (opt.has_value()) {
        walletRecord_ = std::make_shared<Wallet>(*opt);

        editName_->setText(QString::fromStdString(walletRecord_->name));
        comboChain_->setCurrentIndex(walletRecord_->chainType);
        desc_->setPlainText(QString::fromStdString(walletRecord_->description));

        auto addresses
            = globalManager_->settingManager()
                  ->database()
                  ->addressRepo()
                  ->getAllByWallet(static_cast<int>(walletRecord_->id));

        addressView_->load(addresses);
    } else {
        std::cerr << "No wallet found." << std::endl;
    }

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->init(Frameless::Mode::DIALOG);

    globalManager_->windowManager()->reset(this, 0.7);
    connect(frameless_.get(), &Frameless::onMax, this,
        [this]() { globalManager_->windowManager()->reset(this, 0.7); });

    connect(
        ui->ButtonOK, &QPushButton::clicked, this, &UpdateSmartWalletForm::ok);
    connect(ui->ButtonCancel, &QPushButton::clicked, this,
        &UpdateSmartWalletForm::reject);

    connect(ui->ButtonNewAddress, &QPushButton::clicked, this,
        &UpdateSmartWalletForm::newAddress);
    connect(ui->ButtonEditAddress, &QPushButton::clicked, this,
        &UpdateSmartWalletForm::editAddress);

    connect(addressView_, &AddressListView::doubleClicked, this,
        &UpdateSmartWalletForm::editAddress);
    connect(addressView_, &AddressListView::deleteRequested, this,
        &UpdateSmartWalletForm::delAddress);
}

UpdateSmartWalletForm::~UpdateSmartWalletForm()
{
    delete ui;
}

std::shared_ptr<Wallet> UpdateSmartWalletForm::walletRecord() const
{
    return walletRecord_;
}

void UpdateSmartWalletForm::ok()
{
}

void UpdateSmartWalletForm::newAddress()
{
    NewSmartAddressForm::NewAddress address {
        .walletId = walletRecord_->id,
        .groupType = walletRecord_->groupType,
        .chainType = walletRecord_->chainType,
        .index = 0,
        .mnemonic = walletRecord_->mnemonic,
    };
    NewSmartAddressForm naf(address, this, globalManager_);
    int ret = naf.exec();
    if (ret) {
        auto newAddr = *naf.addressRecord();
        addressView_->add(newAddr);
    } else {
    }
}

void UpdateSmartWalletForm::editAddress()
{
}

void UpdateSmartWalletForm::delAddress(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    if (index.row() == 0) {
        MessageForm mf(this, 14, CONFIRM_FIRST_WALLET_DELETE,
            CONFIRM_WALLET_DELETE_TITLE, false, MessageButton::Ok);
        mf.exec();
        return;
    }

    int id
        = index.data(static_cast<int>(AddressListModel::ItemData::Id)).toInt();

    QString name
        = index.data(static_cast<int>(AddressListModel::ItemData::Name))
              .toString();

    MessageForm mf(this, 14, CONFIRM_ADDRESS_DELETE.arg(name),
        CONFIRM_WALLET_DELETE_TITLE, false,
        MessageButton::Ok | MessageButton::Cancel);

    if (mf.exec()) {
        try {
            int addrIndex
                = index
                      .data(static_cast<int>(AddressListModel::ItemData::Index))
                      .toInt();

            globalManager_->settingManager()->database()->addressRepo()->remove(
                id);
            addressView_->remove(QList<int>() << index.row());
        } catch (const DatabaseException& e) {
            std::cerr << "Failed to remove address: " << e.what() << std::endl;
        }
    }
}
