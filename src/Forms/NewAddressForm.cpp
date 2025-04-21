#include "NewAddressForm.h"
#include "ui_NewAddressForm.h"

NewAddressForm::NewAddressForm(const NewAddress& address, QWidget* parent,
    const std::shared_ptr<const GlobalManager>& globalManager)
    : QDialog(parent)
    , ui(new Ui::NewAddressForm)
    , address_(address)
    , globalManager_(globalManager)
{
    ui->setupUi(this);

    QVBoxLayout* layoutOptions = new QVBoxLayout(ui->groupBox);
    layoutOptions->setContentsMargins(DEFAULT_GROUP_MARGINS);
    layoutOptions->setSpacing(DEFAULT_SPACING);

    QLabel* labelName = new QLabel(this);
    labelName->setText(STR_LABEL_NAME);

    editName_ = new LineEditEx(this);
    editName_->setMaxLength(DEFAULT_MAXLENGTH);
    editName_->setPlaceholderText(STR_LINEEDIT_ADDRESS_NAME_PLACEHOLDER);
    editName_->setCursorPosition(0);

    if (address_.groupType == static_cast<int>(WalletGroupType::Import)) {
        // TODO
    }

    QLabel* labelDesc = new QLabel(this);
    labelDesc->setText(STR_LABEL_DESC);

    text_ = new PlainTextEditEx(this);
    text_->setObjectName("plainDesc");

    layoutOptions->addWidget(labelName);
    layoutOptions->addWidget(editName_);
    layoutOptions->addWidget(labelDesc);
    layoutOptions->addWidget(text_, 1);
    // ui->groupBox->setLayout(layoutOptions);

    switch (address_.op) {
    case Op::NEW:
        setWindowTitle(DEFAULT_TITLE_NEW);

        editName_->setText(QString(STR_ADDRESS_NAME).arg(address_.index + 1));

        {
            addressRecord_ = std::make_shared<Address>();
            addressRecord_->walletId = address_.walletId;
            addressRecord_->index = address_.index;
        }

        break;

    case Op::EDIT: {
        setWindowTitle(DEFAULT_TITLE_EDIT);

        auto opt
            = globalManager_->settingManager()->database()->addressRepo()->get(
                address_.id);
        if (opt.has_value()) {
            addressRecord_ = std::make_shared<Address>(*opt);

            editName_->setText(QString::fromStdString(addressRecord_->name));
            text_->setPlainText(
                QString::fromStdString(addressRecord_->description));
        } else {
            std::cerr << "No address found." << std::endl;
        }
        break;
    }

    default:
        break;
    }

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->init(Frameless::Mode::DIALOG);

    globalManager_->windowManager()->reset(
        this, 0.6, WindowManager::WindowShape::SQUARE);
    connect(frameless_.get(), &Frameless::onMax, this, [this]() {
        globalManager_->windowManager()->reset(
            this, 0.6, WindowManager::WindowShape::SQUARE);
    });

    connect(ui->ButtonOK, &QPushButton::clicked, this, &NewAddressForm::ok);
    connect(
        ui->ButtonCancel, &QPushButton::clicked, this, &NewAddressForm::reject);
}

NewAddressForm::~NewAddressForm()
{
    delete ui;
}

std::shared_ptr<Address> NewAddressForm::addressRecord() const
{
    return addressRecord_;
}

void NewAddressForm::ok()
{
    if (editName_->text().isEmpty()) {
        MessageForm { this, 5, NO_VALID_ADDRESS_NAME }.exec();
        return;
    }

    const auto name = simplified(editName_->text().toStdString());
    const auto oldName = simplified(addressRecord_->name);

    const auto desc = trim(text_->toPlainText().toStdString());
    const auto oldDesc = trim(addressRecord_->description);

    auto addressRepo = dynamic_cast<AddressRepo*>(
        globalManager_->settingManager()->database()->addressRepo());

    addressRecord_->nameHash = Encryption::easyHash(name);

    switch (address_.op) {
    case Op::NEW: {
        if (addressRepo->haveName(*addressRecord_)) {
            MessageForm { this, 16, SAME_ADDRESS_NAME }.exec();
            return;
        }

        switch (static_cast<WalletGroupType>(address_.groupType)) {
        case WalletGroupType::User:
        case WalletGroupType::Import: {
            std::unique_ptr<ChainWallet> wallet;
            switch (static_cast<ChainType>(address_.chainType)) {
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

            try {
                Encryption enc;

                const std::string decrypted
                    = enc.decryptText(address_.mnemonic);
                wallet->fromMnemonic(decrypted);
                const auto address = wallet->getAddress(address_.index);
                const std::string addressHash = Encryption::easyHash(address);

                {
                    addressRecord_->walletId = address_.walletId;
                    addressRecord_->hash = Encryption::genRandomHash();
                    addressRecord_->name = name;
                    addressRecord_->address = address;
                    addressRecord_->addressHash = addressHash;
                    addressRecord_->derivationPath
                        = std::string(wallet->getDerivationPath());
                    addressRecord_->privateKey = enc.encryptText(
                        wallet->getPrivateKey(address_.index));
                    addressRecord_->publicKey
                        = wallet->getAddress(address_.index);
                    addressRecord_->description = desc;
                }

                const auto addressId = globalManager_->settingManager()
                                           ->database()
                                           ->addressRepo()
                                           ->insert(*addressRecord_);
                addressRecord_->id = addressId;
                accept();
            } catch (const std::exception& e) {
                std::cerr << "Failed to import menmonic: " << e.what()
                          << std::endl;
            }

            break;
        }

        default:
            break;
        }

        break;
    }

    case Op::EDIT: {
        if (name == oldName && desc == oldDesc) {
            reject();
            return;
        }
        if (name != oldName) {
            if (addressRepo->haveName(*addressRecord_)) {
                MessageForm { this, 16, SAME_WALLET_NAME }.exec();
                return;
            }
        }
        addressRecord_->name = name;
        addressRecord_->description = desc;
        addressRepo->update(*addressRecord_);
        accept();
        break;
    }
    default:
        break;
    }
}
