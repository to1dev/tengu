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
    frameless_->init();

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

    QVBoxLayout* layoutOptioins = new QVBoxLayout(ui->groupBox);
    layoutOptioins->setContentsMargins(DEFAULT_GROUP_MARGINS);
    layoutOptioins->setSpacing(DEFAULT_SPACING);

    QLabel* labelName = new QLabel(this);
    labelName->setText(STR_LABEL_NAME);

    editName_ = new LineEditEx(this);
    editName_->setText(
        QString::fromStdString(NameGenerator(NAME_PATTERN).toString()));
    editName_->setMaxLength(DEFAULT_MAXLENGTH);
    editName_->setPlaceholderText(STR_LINEEDIT_WALLET_NAME_PLACEHOLDER);
    editName_->setCursorPosition(0);

    QPushButton* buttonClipboard = new QPushButton(this);
    buttonClipboard->setObjectName("ButtonClipboard");
    buttonClipboard->setText(STR_BUTTON_CLIPBOARD);
    connect(buttonClipboard, &QPushButton::clicked,
        [this]() { QApplication::clipboard()->setText(view_->mnemonic()); });

    layoutOptioins->addWidget(labelName);
    layoutOptioins->addWidget(editName_);
    layoutOptioins->addStretch(1);
    layoutOptioins->addWidget(buttonClipboard);
    ui->groupBox->setLayout(layoutOptioins);

    ui->ButtonOK->setDefault(true);

    windowManager_->setWindow(this);
    windowManager_->reset(0.6);

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
        MessageForm mf(this, NO_VALID_WALLET_NAME);
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
            walletRecord_->id = -1;
            walletRecord_->type = 0;
            walletRecord_->groupId = 0;
            walletRecord_->hash = Encryption::genRandomHash();
            walletRecord_->name = name.toStdString();
            walletRecord_->mnemonic = encrypted;
        }

        auto walletId = globalManager_->settingManager()
                            ->database()
                            ->walletRepo()
                            ->insert(*walletRecord_);

        std::string address = wallet.getAddress();
        std::string addressName = std::string(STR_DEFAULT_ADDRESS_NAME);
        std::string addressNameHash = Encryption::easyHash(addressName);
        std::string addressHash = Encryption::easyHash(address);

        Address addressRecord {
            .id = -1,
            .type = 0,
            .walletId = walletId,
            .chainType = static_cast<int>(ChainType::SOLANA),
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
            MessageForm mf(this, SAME_WALLET_NAME);
            mf.exec();
            break;
        }

        case DBErrorType::haveMnemonic: {
            MessageForm mf(this, SAME_MNEMONIC);
            mf.exec();
            break;
        }

        default:
            break;
        }
    }
}
