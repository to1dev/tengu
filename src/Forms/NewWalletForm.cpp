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

NewWalletForm::NewWalletForm(const NewWallet& wallet, QWidget* parent,
    const std::shared_ptr<const GlobalManager>& globalManager)
    : QDialog(parent)
    , ui(new Ui::NewWalletForm)
    , wallet_(wallet)
    , globalManager_(globalManager)
    , walletRecord_(std::make_shared<Wallet>())
{
    ui->setupUi(this);

    setWindowTitle(DEFAULT_TITLE);

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
    // ui->groupBoxMnemonic->setLayout(layoutView);

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
        comboChain_->addItem(QString::fromUtf8(
            chain.second.name.data(), chain.second.name.size()));
        bool enabled = chain.second.enabled;
        if (!enabled) {
            comboChain_->setItemEnabled(index, false);
        }
        index++;
    }
    comboChain_->setCurrentIndex(0);

    QLabel* labelDesc = new QLabel(this);
    labelDesc->setText(STR_LABEL_DESC);

    text_ = new PlainTextEditEx(this);
    text_->setObjectName("plainDesc");

    QPushButton* buttonClipboard = new QPushButton(this);
    buttonClipboard->setObjectName("ButtonClipboard");
    buttonClipboard->setText(STR_BUTTON_CLIPBOARD);
    buttonClipboard->setMinimumHeight(30);
    buttonClipboard->setMaximumHeight(30);
    connect(buttonClipboard, &QPushButton::clicked,
        [this]() { clip::set_text(view_->mnemonic().toStdString()); });

    layoutOptions->addWidget(labelName);
    layoutOptions->addWidget(editName_);
    layoutOptions->addWidget(labelChain);
    layoutOptions->addWidget(comboChain_);
    layoutOptions->addWidget(labelDesc);
    layoutOptions->addWidget(text_, 1);
    layoutOptions->addSpacing(10);
    layoutOptions->addWidget(buttonClipboard);
    // ui->groupBox->setLayout(layoutOptions);

    ui->ButtonOK->setDefault(true);

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->init(Frameless::Mode::DIALOG);

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
        MessageForm { this, 5, NO_VALID_WALLET_NAME }.exec();
        return;
    }

    const QString name = editName_->text().simplified();
    const QString mnemonic = view_->mnemonic().simplified();

    const auto nameHash = Encryption::easyHash(name);
    const auto mnemonicHash = Encryption::easyHash(mnemonic);

    walletRecord_->nameHash = nameHash.toStdString();
    walletRecord_->mnemonicHash = mnemonicHash.toStdString();

    auto walletRepo = dynamic_cast<WalletRepo*>(
        globalManager_->settingManager()->database()->walletRepo());

    if (walletRepo->haveName(*walletRecord_)) {
        MessageForm { this, 16, SAME_WALLET_NAME }.exec();
        return;
    }

    if (walletRepo->haveMnemonic(*walletRecord_)) {
        MessageForm { this, 16, SAME_MNEMONIC }.exec();
        return;
    }

    std::unique_ptr<ChainWallet> wallet;
    switch (static_cast<ChainType>(comboChain_->currentIndex())) {
    case ChainType::BITCOIN:
        wallet = std::make_unique<BitcoinWallet>();
        break;
    case ChainType::ETHEREUM:
        wallet = std::make_unique<EthereumWallet>();
        break;
    case ChainType::SOLANA:
        wallet = std::make_unique<SolanaWallet>();
        break;
    default:
        MessageForm { this, 16, "Unsupported chain type" }.exec();
        return;
    }

    Encryption enc;

    wallet->fromMnemonic(mnemonic.toStdString());
    const auto encrypted = enc.encryptText(wallet->mnemonic());
    {
        walletRecord_->type = static_cast<int>(WalletType::Mnemonic);
        walletRecord_->groupType = static_cast<int>(WalletGroupType::User);
        walletRecord_->chainType = comboChain_->currentIndex(),
        walletRecord_->hash = Encryption::genRandomHash();
        walletRecord_->name = name.toStdString();
        walletRecord_->mnemonic = encrypted;
        walletRecord_->description = trim(text_->toPlainText().toStdString());
    }

    auto database = globalManager_->settingManager()->database();

    bool success = database->storage()->transaction([&]() {
        const int walletId = database->walletRepo()->insert(*walletRecord_);

        if (walletId <= 0) {
            return false;
        }

        walletRecord_->id = walletId;

        const auto address = wallet->getAddress();
        const std::string addressName = STR_DEFAULT_ADDRESS_NAME;
        const auto addressNameHash = Encryption::easyHash(addressName);
        const auto addressHash = Encryption::genRandomHash();

        Address addressRecord {
            .walletId = walletId,
            .hash = Encryption::genRandomHash(),
            .name = addressName,
            .nameHash = addressNameHash,
            .address = address,
            .addressHash = addressHash,
            .derivationPath = std::string(wallet->getDerivationPath()),
            .privateKey = enc.encryptText(wallet->getPrivateKey()),
            .publicKey = wallet->getAddress(),
        };

        if (ChainType::BITCOIN
            == static_cast<ChainType>(walletRecord_->chainType)) {
            addressRecord.type = static_cast<int>(AddressType::Taproot);
        }

        if (database->addressRepo()->insert(addressRecord) <= 0) {
            return false;
        }

        return true;
    });

    if (!success) {
        MessageForm { this, 16, "Failed to create wallet and address" }.exec();
        return;
    }

    accept();
}
