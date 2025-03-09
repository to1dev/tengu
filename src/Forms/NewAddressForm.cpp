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
        setWindowTitle(DEFAULT_TITLE_NEW);
        editName_->setText(QString(STR_ADDRESS_NAME).arg(address_.count + 1));

        {
            addressRecord_ = std::make_shared<Address>();
            addressRecord_->id = address_.id;
            addressRecord_->walletId = address_.walletId;
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

    switch (address_.op) {
    case Op::NEW: {
        addressRecord_->nameHash = Encryption::easyHash(name.toStdString());
        DBErrorType error = globalManager_->settingManager()
                                ->database()
                                ->addressRepo()
                                ->before(*addressRecord_, true);

        if (error != DBErrorType::none) {
            switch (error) {
            case DBErrorType::haveName:
                MessageForm { this, 16, SAME_ADDRESS_NAME }.exec();
                break;
            default:
                break;
            }
            return;
        }

        break;
    }

    case Op::EDIT: {
        if (name != oldName) {
            addressRecord_->nameHash = Encryption::easyHash(name.toStdString());

            DBErrorType error = globalManager_->settingManager()
                                    ->database()
                                    ->addressRepo()
                                    ->before(*addressRecord_, true);

            if (error != DBErrorType::none) {
                switch (error) {
                case DBErrorType::haveName:
                    MessageForm { this, 16, SAME_WALLET_NAME }.exec();
                    break;
                default:
                    break;
                }
                return;
            }

            addressRecord_->name = name.toStdString();
            globalManager_->settingManager()->database()->addressRepo()->update(
                *addressRecord_);

            accept();
        } else {
            reject();
        }

        break;
    }

    default:
        break;
    }
}
