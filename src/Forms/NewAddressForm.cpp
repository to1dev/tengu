#include "NewAddressForm.h"
#include "ui_NewAddressForm.h"

NewAddressForm::NewAddressForm(const _Address& address, QWidget* parent,
    const std::shared_ptr<const GlobalManager>& globalManager)
    : QDialog(parent)
    , ui(new Ui::NewAddressForm)
    , address_(address)
    , globalManager_(globalManager)
{
    ui->setupUi(this);

    switch (address_.op) {
    case Op::NEW: {
        setWindowTitle(DEFAULT_TITLE_NEW);
        break;
    }
    case Op::EDIT: {
        setWindowTitle(DEFAULT_TITLE_EDIT);
        break;
    }
    default:
        break;
    }

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->init(Frameless::Mode::DIALOG);

    QVBoxLayout* layoutOptions = new QVBoxLayout(ui->groupBox);
    layoutOptions->setContentsMargins(DEFAULT_GROUP_MARGINS);
    layoutOptions->setSpacing(DEFAULT_SPACING);

    QLabel* labelName = new QLabel(this);
    labelName->setText(STR_LABEL_NAME);

    editName_ = new LineEditEx(this);
    editName_->setMaxLength(DEFAULT_MAXLENGTH);
    editName_->setPlaceholderText(STR_LINEEDIT_ADDRESS_NAME_PLACEHOLDER);
    editName_->setCursorPosition(0);

    if (address_.groupType == static_cast<int>(WalletGroupType::Vault)) {
        // TODO
    }

    layoutOptions->addWidget(labelName);
    layoutOptions->addWidget(editName_);
    layoutOptions->addStretch(1);
    ui->groupBox->setLayout(layoutOptions);

    globalManager_->windowManager()->reset(
        this, 0.6, WindowManager::WindowShape::SQUARE);
    connect(frameless_.get(), &Frameless::onMax, this, [this]() {
        globalManager_->windowManager()->reset(
            this, 0.6, WindowManager::WindowShape::SQUARE);
    });

    connect(ui->ButtonOK, &QPushButton::clicked, this, &NewAddressForm::ok);
    connect(
        ui->ButtonCancel, &QPushButton::clicked, this, &NewAddressForm::reject);

    switch (address_.op) {
    case Op::NEW:
        editName_->setText(QString(STR_ADDRESS_NAME).arg(address_.count + 1));

        {
            addressRecord_ = std::make_shared<Address>();
            addressRecord_->id = address_.id;
            addressRecord_->walletId = address_.walletId;
        }

        break;

    case Op::EDIT: {
        auto opt
            = globalManager_->settingManager()->database()->addressRepo()->get(
                address_.id);
        if (opt.has_value()) {
            addressRecord_ = std::make_shared<Address>(*opt);

            editName_->setText(QString::fromStdString(addressRecord_->name));
        } else {
            std::cerr << "No address found." << std::endl;
        }
        break;
    }

    default:
        break;
    }
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

    const QString name = editName_->text().simplified();
    const QString oldName = addressRecord_
        ? QString::fromStdString(addressRecord_->name).simplified()
        : "";

    auto addressRepo
        = globalManager_->settingManager()->database()->addressRepo();

    auto checkNameAndReturnError = [&]() -> DBErrorType {
        addressRecord_->nameHash = Encryption::easyHash(name.toStdString());
        return addressRepo->before(*addressRecord_, true);
    };

    switch (address_.op) {
    case Op::NEW: {
        DBErrorType error = checkNameAndReturnError();
        if (error != DBErrorType::none) {
            if (error == DBErrorType::haveName) {
                MessageForm { this, 16, SAME_ADDRESS_NAME }.exec();
            }
            return;
        }

        if (address_.groupType == static_cast<int>(WalletGroupType::User)) {
            std::unique_ptr<ChainWallet> wallet;
            switch (address_.chainType) {
            case 0:
                wallet = std::make_unique<BitcoinWallet>();
                break;
            case 1:
                wallet = std::make_unique<EthereumWallet>();
                break;
            case 2:
                wallet = std::make_unique<SolanaWallet>();
                break;
            default:
                MessageForm { this, 16, "Unsupported chain type" }.exec();
                return;
            }
            std::cout << address_.chainType << std::endl;
        }

        break;
    }
    case Op::EDIT: {
        if (name == oldName) {
            reject();
            return;
        }
        DBErrorType error = checkNameAndReturnError();
        if (error != DBErrorType::none) {
            if (error == DBErrorType::haveName) {
                MessageForm { this, 16, SAME_WALLET_NAME }.exec();
            }
            return;
        }
        addressRecord_->name = name.toStdString();
        addressRepo->update(*addressRecord_);
        accept();
        break;
    }
    default:
        break;
    }
}
