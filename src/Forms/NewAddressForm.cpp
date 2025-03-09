#include "NewAddressForm.h"
#include "ui_NewAddressForm.h"

NewAddressForm::NewAddressForm(QWidget* parent,
    const std::shared_ptr<const GlobalManager>& globalManager,
    const AddressOp& op)
    : QDialog(parent)
    , ui(new Ui::NewAddressForm)
    , globalManager_(globalManager)
    , op_(op)
{
    ui->setupUi(this);

    switch (op_) {
    case AddressOp::NEW:
        setWindowTitle(DEFAULT_TITLE_NEW);
        break;

    case AddressOp::EDIT:
        setWindowTitle(DEFAULT_TITLE_EDIT);
        break;

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
}

NewAddressForm::~NewAddressForm()
{
    delete ui;
}

void NewAddressForm::setId(int id)
{
    switch (op_) {
    case AddressOp::NEW:
        break;

    case AddressOp::EDIT: {
        auto opt
            = globalManager_->settingManager()->database()->addressRepo()->get(
                id);
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

std::shared_ptr<Address> NewAddressForm::addressRecord() const
{
    return std::shared_ptr<Address>();
}

void NewAddressForm::ok()
{
    accept();
}
