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

#include "ImportWalletForm.h"
#include "ui_ImportWalletForm.h"

ImportWalletForm::ImportWalletForm(
    QWidget* parent, const std::shared_ptr<const GlobalManager>& globalManager)
    : QDialog(parent)
    , ui(new Ui::ImportWalletForm)
    , globalManager_(globalManager)
    , walletRecord_(std::make_shared<Wallet>())
{
    ui->setupUi(this);

    setWindowTitle(DEFAULT_TITLE);

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->init(Frameless::Mode::DIALOG);

    QVBoxLayout* layout = new QVBoxLayout(ui->groupBox);
    layout->setContentsMargins(DEFAULT_GROUP_MARGINS);
    layout->setSpacing(DEFAULT_SPACING);

    text_ = new CryptoTextEdit(this);
    text_->setFocus();

    layout->addWidget(text_);
    // ui->groupBox->setLayout(layout);

    QVBoxLayout* layoutOptions = new QVBoxLayout(ui->groupBoxOptions);
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
    comboChain_->setCurrentIndex(0);

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
    // ui->groupBoxOptions->setLayout(layoutOptions);

    ui->ButtonOK->setDefault(true);

    globalManager_->windowManager()->reset(this, 0.7);
    connect(frameless_.get(), &Frameless::onMax, this,
        [this]() { globalManager_->windowManager()->reset(this, 0.7); });

    connect(ui->ButtonOK, &QPushButton::clicked, this, &ImportWalletForm::ok);
    connect(ui->ButtonCancel, &QPushButton::clicked, this,
        &ImportWalletForm::reject);
    connect(ui->ButtonRefresh, &QPushButton::clicked, [&]() {
        editName_->setText(
            QString::fromStdString(NameGenerator(NAME_PATTERN).toString()));
    });

    connect(text_, &CryptoTextEdit::contentDetected,
        [this](const ContentInfo& info) {
            currentContent_ = info;
            if (info.type == WalletType::Mnemonic) {
                comboChain_->setEnabled(true);
            } else {
                comboChain_->setEnabled(false);
            }
            int chain = static_cast<int>(currentContent_.chain);
            if (chain >= 0) {
                comboChain_->setCurrentIndex(chain);
            }
        });
}

ImportWalletForm::~ImportWalletForm()
{
    delete ui;
}

std::shared_ptr<Wallet> ImportWalletForm::walletRecord() const
{
    return walletRecord_;
}

void ImportWalletForm::ok()
{
    if (editName_->text().isEmpty()) {
        MessageForm { this, 5, NO_VALID_WALLET_NAME }.exec();
        return;
    }

    if (!currentContent_.isValid()) {
        MessageForm { this, 5, NO_VALID_MNEMONIC_KEY }.exec();
        return;
    }

    const QString name = editName_->text().simplified();
    const auto nameHash = Encryption::easyHash(name);

    walletRecord_->nameHash = nameHash.toStdString();

    if (currentContent_.type == WalletType::Mnemonic) {
        currentContent_.chain
            = static_cast<ChainType>(comboChain_->currentIndex());
    }

    const QString mnemonic = currentContent_.content;
    const auto mnemonicHash = Encryption::easyHash(mnemonic);
    walletRecord_->mnemonicHash = mnemonicHash.toStdString();

    std::unique_ptr<ChainWallet> wallet;
    switch (currentContent_.chain) {
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

    DBErrorType error
        = globalManager_->settingManager()->database()->walletRepo()->before(
            *walletRecord_);
    if (error != DBErrorType::none) {
        switch (error) {
        case DBErrorType::haveName:
            MessageForm { this, 16, SAME_WALLET_NAME }.exec();
            break;
        case DBErrorType::haveMnemonic:
            switch (currentContent_.type) {
            case WalletType::Priv:
            case WalletType::Wif:
                MessageForm { this, 16, SAME_PRIV }.exec();
                break;

            case WalletType::Address:
                MessageForm { this, 16, SAME_ADDRESS }.exec();
                break;

            case WalletType::Mnemonic:
            default:
                MessageForm { this, 16, SAME_MNEMONIC }.exec();
                break;
            }

            break;
        default:
            break;
        }
        return;
    }

    walletRecord_->type = static_cast<int>(currentContent_.type);
    walletRecord_->groupType = static_cast<int>(WalletGroupType::Import);
    walletRecord_->hash = Encryption::genRandomHash();
    walletRecord_->name = name.toStdString();
    walletRecord_->description = trim(desc_->toPlainText().toStdString());

    switch (currentContent_.type) {
    case (WalletType::Mnemonic): {
        try {
            wallet->fromMnemonic(currentContent_.content.toStdString());
            const auto encrypted = Encryption::encryptText(wallet->mnemonic());
            {
                walletRecord_->chainType = comboChain_->currentIndex();
                walletRecord_->mnemonic = encrypted;
            }

            auto database = globalManager_->settingManager()->database();
            bool success = database->storage()->transaction([&]() {
                const int walletId
                    = database->walletRepo()->insert(*walletRecord_);

                if (walletId <= 0) {
                    return false;
                }

                walletRecord_->id = walletId;

                const auto address = wallet->getAddress();
                const std::string addressName = STR_DEFAULT_ADDRESS_NAME;
                const auto addressNameHash = Encryption::easyHash(addressName);
                const auto addressHash = Encryption::easyHash(address);

                Address addressRecord {
                    .walletId = walletId,
                    .hash = Encryption::genRandomHash(),
                    .name = addressName,
                    .nameHash = addressNameHash,
                    .address = address,
                    .addressHash = addressHash,
                    .derivationPath = std::string(wallet->getDerivationPath()),
                    .privateKey
                    = Encryption::encryptText(wallet->getPrivateKey()),
                    .publicKey = wallet->getAddress(),
                };

                if (database->addressRepo()->insert(addressRecord) <= 0) {
                    return false;
                }

                return true;
            });

            if (!success) {
                MessageForm { this, 16, "Failed to create wallet and address" }
                    .exec();
                return;
            }

            accept();
        } catch (const std::exception& e) {
            std::cerr << "Failed to import Mnemonic: " << e.what() << std::endl;
            MessageForm { this, 5, NO_VALID_MNEMONIC_KEY }.exec();
            return;
        }

        break;
    }

    case WalletType::Priv:
    case WalletType::Wif: {
        try {
            wallet->fromPrivateKey(currentContent_.content.toStdString());
            const auto encrypted
                = Encryption::encryptText(currentContent_.content);
            {
                walletRecord_->chainType
                    = static_cast<int>(currentContent_.chain);
            }

            auto database = globalManager_->settingManager()->database();
            bool success = database->storage()->transaction([&]() {
                const int walletId
                    = database->walletRepo()->insert(*walletRecord_);

                if (walletId <= 0) {
                    return false;
                }

                walletRecord_->id = walletId;

                const auto address = wallet->getAddress();
                const std::string addressName = STR_DEFAULT_ADDRESS_NAME;
                const auto addressNameHash = Encryption::easyHash(addressName);
                const auto addressHash = Encryption::easyHash(address);

                Address addressRecord {
                    .walletId = walletId,
                    .hash = Encryption::genRandomHash(),
                    .name = addressName,
                    .nameHash = addressNameHash,
                    .address = address,
                    .addressHash = addressHash,
                    .derivationPath = std::string(wallet->getDerivationPath()),
                    .privateKey = encrypted.toStdString(),
                    .publicKey = wallet->getAddress(),
                };

                if (database->addressRepo()->insert(addressRecord) <= 0) {
                    return false;
                }

                return true;
            });

            if (!success) {
                MessageForm { this, 16, "Failed to create wallet and address" }
                    .exec();
                return;
            }

            accept();
        } catch (const std::exception& e) {
            std::cerr << "Failed to import private key: " << e.what()
                      << std::endl;
            MessageForm { this, 5, NO_VALID_MNEMONIC_KEY }.exec();
            return;
        }

        break;
    }

    case (WalletType::Address): {
        try {
            {
                walletRecord_->chainType
                    = static_cast<int>(currentContent_.chain);
            }

            auto database = globalManager_->settingManager()->database();
            bool success = database->storage()->transaction([&]() {
                const int walletId
                    = database->walletRepo()->insert(*walletRecord_);

                if (walletId <= 0) {
                    return false;
                }

                walletRecord_->id = walletId;

                const auto address = currentContent_.content.toStdString();
                const std::string addressName = STR_DEFAULT_ADDRESS_NAME;
                const auto addressNameHash = Encryption::easyHash(addressName);
                const auto addressHash = Encryption::easyHash(address);

                Address addressRecord {
                    .walletId = walletId,
                    .hash = Encryption::genRandomHash(),
                    .name = addressName,
                    .nameHash = addressNameHash,
                    .address = address,
                    .addressHash = addressHash,
                    .derivationPath = std::string(wallet->getDerivationPath()),
                };

                if (database->addressRepo()->insert(addressRecord) <= 0) {
                    return false;
                }

                return true;
            });

            if (!success) {
                MessageForm { this, 16, "Failed to create wallet and address" }
                    .exec();
                return;
            }

            accept();
        } catch (const std::exception& e) {
            std::cerr << "Failed to import address: " << e.what() << std::endl;
            MessageForm { this, 5, NO_VALID_MNEMONIC_KEY }.exec();
            return;
        }

        break;
    }

    default:
        break;
    }
}
