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

#include "NewWalletForm.h"
#include "ui_NewWalletForm.h"

NewWalletForm::NewWalletForm(
    QWidget* parent, const std::shared_ptr<const GlobalManager>& globalManager)
    : QDialog(parent)
    , ui(new Ui::NewWalletForm)
    , globalManager_(globalManager)
{
    ui->setupUi(this);

    setWindowTitle(DEFAULT_TITLE);

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->init(Frameless::Mode::DIALOG);

    view_ = new MnemonicView(this);
    try {
        std::string mnemonic = BaseMnemonic::generate();
        view_->setMnemonic(QString::fromStdString(mnemonic));
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
    } catch (const MnemonicException& e) {
        std::cerr << "Mnemonic generation failed: " << e.what() << std::endl;
    }

    QVBoxLayout* layoutView = new QVBoxLayout(ui->groupBoxMnemonic);
    layoutView->setContentsMargins(DEFAULT_GROUP_MARGINS);
    layoutView->setSpacing(DEFAULT_SPACING);
    layoutView->addWidget(view_);
    ui->groupBoxMnemonic->setLayout(layoutView);

    QVBoxLayout* layoutOptions = new QVBoxLayout(ui->groupBox);
    layoutOptions->setContentsMargins(DEFAULT_GROUP_MARGINS);
    layoutOptions->setSpacing(DEFAULT_SPACING);

    QLabel* labelName = new QLabel(this);
    labelName->setText(STR_LABEL_NAME);

    editName_ = new LineEditEx(this);
    editName_->setText(
        QString::fromStdString(NameGenerator(NAME_PATTERN).toString()));
    editName_->setMaxLength(DEFAULT_MAXLENGTH);
    editName_->setPlaceholderText(STR_LINEEDIT_WALLET_NAME_PLACEHOLDER);
    editName_->setCursorPosition(0);

    QLabel* labelChain = new QLabel(this);
    labelChain->setText(STR_LABEL_CHAIN);

    comboChain_ = new ComboBoxEx(this);
    int index = 0;
    for (const auto& chain : Chains) {
        comboChain_->addItem(QString::fromUtf8(chain.second.name.data()));
        bool enabled = chain.second.enabled;
        if (!enabled) {
            comboChain_->setItemEnabled(index, false);
        }
        index++;
    }
    comboChain_->setCurrentIndex(0);

    QPushButton* buttonClipboard = new QPushButton(this);
    buttonClipboard->setObjectName("ButtonClipboard");
    buttonClipboard->setText(STR_BUTTON_CLIPBOARD);
    buttonClipboard->setMinimumHeight(30);
    buttonClipboard->setMaximumHeight(30);
    connect(buttonClipboard, &QPushButton::clicked,
        [this]() { QApplication::clipboard()->setText(view_->mnemonic()); });

    layoutOptions->addWidget(labelName);
    layoutOptions->addWidget(editName_);
    layoutOptions->addWidget(labelChain);
    layoutOptions->addWidget(comboChain_);
    layoutOptions->addStretch(1);
    layoutOptions->addWidget(buttonClipboard);
    ui->groupBox->setLayout(layoutOptions);

    ui->ButtonOK->setDefault(true);

    globalManager_->windowManager()->reset(this, 0.7);
    connect(frameless_.get(), &Frameless::onMax, this,
        [this]() { globalManager_->windowManager()->reset(this, 0.7); });

    connect(ui->ButtonOK, &QPushButton::clicked, this, &NewWalletForm::ok);
    connect(
        ui->ButtonCancel, &QPushButton::clicked, this, &NewWalletForm::reject);
    connect(ui->ButtonRefresh, &QPushButton::clicked, this,
        &NewWalletForm::refresh);
}

NewWalletForm::~NewWalletForm()
{
    delete ui;
}

std::shared_ptr<Wallet> NewWalletForm::walletRecord() const
{
    return walletRecord_;
}

void NewWalletForm::refresh()
{
    QObject* obj = sender();

    if (obj == ui->ButtonRefresh) {
        try {
            std::string mnemonic = BaseMnemonic::generate();
            view_->setMnemonic(QString::fromStdString(mnemonic));
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid argument: " << e.what() << std::endl;
        } catch (const MnemonicException& e) {
            std::cerr << "Mnemonic generation failed: " << e.what()
                      << std::endl;
        }

        editName_->setText(
            QString::fromStdString(NameGenerator(NAME_PATTERN).toString()));
    }
}

void NewWalletForm::ok()
{
    if (editName_->text().isEmpty()) {
        MessageForm mf(this, 5, NO_VALID_WALLET_NAME);
        mf.exec();
        return;
    }

    QString name = editName_->text().simplified();
    QString mnemonic = view_->mnemonic().simplified();

    QString nameHash = Encryption::easyHash(name);
    QString mnemonicHash = Encryption::easyHash(mnemonic);

    walletRecord_ = std::make_shared<Wallet>();
    walletRecord_->nameHash = nameHash.toStdString();
    walletRecord_->mnemonicHash = mnemonicHash.toStdString();
    DBErrorType error
        = globalManager_->settingManager()->database()->walletRepo()->before(
            *walletRecord_);
    if (DBErrorType::none == error) {
        SolanaWallet wallet;
        wallet.fromMnemonic(mnemonic.toStdString());
        std::string encrypted = Encryption::encryptText(wallet.mnemonic());
        {
            walletRecord_->chainType = comboChain_->currentIndex(),
            walletRecord_->hash = Encryption::genRandomHash();
            walletRecord_->name = name.toStdString();
            walletRecord_->mnemonic = encrypted;
        }

        auto walletId = globalManager_->settingManager()
                            ->database()
                            ->walletRepo()
                            ->insert(*walletRecord_);

        walletRecord_->id = walletId;
        std::string address = wallet.getAddress();
        std::string addressName = std::string(STR_DEFAULT_ADDRESS_NAME);
        std::string addressNameHash = Encryption::easyHash(addressName);
        std::string addressHash = Encryption::easyHash(address);

        Address addressRecord {
            .walletId = walletId,
            .hash = Encryption::genRandomHash(),
            .name = addressName,
            .nameHash = addressNameHash,
            .address = address,
            .addressHash = addressHash,
            .derivationPath
            = std::string(SolanaWallet::DEFAULT_DERIVATION_PATH),
            .privateKey = Encryption::encryptText(wallet.getPrivateKey()),
            .publicKey = wallet.getAddress(),
        };

        globalManager_->settingManager()->database()->addressRepo()->insert(
            addressRecord);

        accept();
    } else {
        switch (error) {
        case DBErrorType::haveName: {
            MessageForm mf(this, 16, SAME_WALLET_NAME);
            mf.exec();
            break;
        }

        case DBErrorType::haveMnemonic: {
            MessageForm mf(this, 16, SAME_MNEMONIC);
            mf.exec();
            break;
        }

        default:
            break;
        }
    }
}
