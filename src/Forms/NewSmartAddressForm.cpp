#include "NewSmartAddressForm.h"
#include "ui_NewSmartAddressForm.h"

NewSmartAddressForm::NewSmartAddressForm(const NewAddress& address,
    QWidget* parent, const std::shared_ptr<const GlobalManager>& globalManager)
    : QDialog(parent)
    , ui(new Ui::NewSmartAddressForm)
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

        editName_->setText(
            QString::fromStdString(NameGenerator(NAME_PATTERN).toString()));

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

    connect(
        ui->ButtonOK, &QPushButton::clicked, this, &NewSmartAddressForm::ok);
    connect(ui->ButtonCancel, &QPushButton::clicked, this,
        &NewSmartAddressForm::reject);
}

NewSmartAddressForm::~NewSmartAddressForm()
{
    delete ui;
}

std::shared_ptr<Address> NewSmartAddressForm::addressRecord() const
{
    return addressRecord_;
}

void NewSmartAddressForm::ok()
{
    if (editName_->text().isEmpty()) {
        MessageForm { this, 5, NO_VALID_ADDRESS_NAME }.exec();
        return;
    }

    const auto name = simplified(editName_->text().toStdString());
    const auto oldName = simplified(addressRecord_->name);

    const auto desc = trim(text_->toPlainText().toStdString());
    const auto oldDesc = trim(addressRecord_->description);

    auto addressRepo
        = globalManager_->settingManager()->database()->addressRepo();

    auto checkNameAndReturnError = [&]() -> DBErrorType {
        addressRecord_->nameHash = Encryption::easyHash(name);
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

        break;
    }

    case Op::EDIT: {
        if (name == oldName && desc == oldDesc) {
            reject();
            return;
        }
        if (name != oldName) {
            DBErrorType error = checkNameAndReturnError();
            if (error != DBErrorType::none) {
                if (error == DBErrorType::haveName) {
                    MessageForm { this, 16, SAME_WALLET_NAME }.exec();
                }
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
