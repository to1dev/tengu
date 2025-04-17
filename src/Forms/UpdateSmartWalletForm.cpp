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
