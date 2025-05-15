#include "NewSmartAddressForm.h"
#include "ui_NewSmartAddressForm.h"

namespace {
inline const QString NO_VALID_ADDRESSES = QObject::tr("请输入有效的地址列表！");
}

NewSmartAddressForm::NewSmartAddressForm(const NewAddress& address,
    QWidget* parent, const std::shared_ptr<const GlobalManager>& globalManager)
    : QDialog(parent)
    , ui(new Ui::NewSmartAddressForm)
    , address_(address)
    , globalManager_(globalManager)
{
    ui->setupUi(this);

    bool isEdit = address_.op == Op::EDIT;

    QVBoxLayout* layoutOptions = new QVBoxLayout(ui->groupBoxOptions);
    layoutOptions->setContentsMargins(DEFAULT_GROUP_MARGINS);
    layoutOptions->setSpacing(DEFAULT_SPACING);

    QLabel* labelName = nullptr;
    QLabel* labelAddress = nullptr;
    if (isEdit) {
        labelName = new QLabel(this);
        labelName->setText(STR_LABEL_NAME);

        editName_ = new LineEditEx(this);
        editName_->setMaxLength(DEFAULT_MAXLENGTH);
        editName_->setPlaceholderText(STR_LINEEDIT_ADDRESS_NAME_PLACEHOLDER);
        editName_->setCursorPosition(0);

        labelAddress = new QLabel(this);
        labelAddress->setText(STR_LABEL_ADDRESS);

        editAddress_ = new LineEditEx(this);
        editAddress_->setMaxLength(DEFAULT_ADDRESS_MAXLENGTH);
        editAddress_->setPlaceholderText(STR_LINEEDIT_ADDRESS_PLACEHOLDER);
        editAddress_->setCursorPosition(0);
    }

    QLabel* labelDesc = new QLabel(this);
    labelDesc->setText(STR_LABEL_DESC);

    desc_ = new PlainTextEditEx(this);
    desc_->setObjectName("plainDesc");

    if (isEdit) {
        layoutOptions->addWidget(labelName);
        layoutOptions->addWidget(editName_);

        layoutOptions->addWidget(labelAddress);
        layoutOptions->addWidget(editAddress_);
    }

    layoutOptions->addWidget(labelDesc);
    layoutOptions->addWidget(desc_, 1);
    // ui->groupBox->setLayout(layoutOptions);

    if (isEdit) {
        setWindowTitle(DEFAULT_TITLE_EDIT);

        ui->groupBoxAddresses->setVisible(false);

        auto opt
            = globalManager_->settingManager()->database()->addressRepo()->get(
                address_.id);
        if (opt.has_value()) {
            addressRecord_ = std::make_shared<Address>(*opt);

            editName_->setText(QString::fromStdString(addressRecord_->name));
            editAddress_->setText(
                QString::fromStdString(addressRecord_->address));
            desc_->setPlainText(
                QString::fromStdString(addressRecord_->description));
        } else {
            std::cerr << "No address found." << std::endl;
        }

    } else {
        setWindowTitle(DEFAULT_TITLE_NEW);

        QVBoxLayout* layout = new QVBoxLayout(ui->groupBoxAddresses);
        layout->setContentsMargins(DEFAULT_GROUP_MARGINS);
        layout->setSpacing(DEFAULT_SPACING);

        text_ = new CryptoAddressEdit(this);
        text_->setFocus();

        layout->addWidget(text_);

        {
            addressRecord_ = std::make_shared<Address>();
            addressRecord_->walletId = address_.walletId;
            addressRecord_->index = address_.index;
        }
    }

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->init(Frameless::Mode::DIALOG);

    WindowManager::WindowShape shape = isEdit
        ? WindowManager::WindowShape::SQUARE
        : WindowManager::WindowShape::HORIZONTAL;

    globalManager_->windowManager()->reset(this, 0.6, shape);
    connect(frameless_.get(), &Frameless::onMax, this, [&, shape]() {
        globalManager_->windowManager()->reset(this, 0.6, shape);
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

const std::vector<Address>& NewSmartAddressForm::addresses() const
{
    return addresses_;
}

void NewSmartAddressForm::ok()
{
    bool isEdit = address_.op == Op::EDIT;
    ChainType type = static_cast<ChainType>(address_.chainType);

    auto addressRepo = dynamic_cast<AddressRepo*>(
        globalManager_->settingManager()->database()->addressRepo());

    if (isEdit) {
        if (editName_->text().isEmpty()) {
            MessageForm { this, 5, NO_VALID_ADDRESS_NAME }.exec();
            return;
        }

        const auto name = simplified(editName_->text().toStdString());
        const auto oldName = simplified(addressRecord_->name);

        const auto desc = trim(desc_->toPlainText().toStdString());
        const auto oldDesc = trim(addressRecord_->description);

        if (editAddress_->text().isEmpty()) {
            MessageForm { this, 5, NO_VALID_ADDRESS }.exec();
            return;
        }

        const auto address = simplified(editAddress_->text().toStdString());
        const auto oldAddress = simplified(addressRecord_->address);

        if (name == oldName && desc == oldDesc && address == oldAddress) {
            reject();
            return;
        }

        addressRecord_->nameHash = Encryption::easyHash(name);
        addressRecord_->addressHash = Encryption::easyHash(address);

        if (name != oldName) {
            if (addressRepo->haveName(*addressRecord_)) {
                MessageForm { this, 16, SAME_WALLET_NAME }.exec();
                return;
            }
        }

        if (address != oldAddress) {
            if (addressRepo->haveAddress(*addressRecord_)) {
                MessageForm { this, 16, SAME_ADDRESS }.exec();
                return;
            }
        }

        auto isValidAddress = [&]() -> bool {
            bool valid = false;
            switch (type) {
            case ChainType::BITCOIN:
                valid = BitcoinWallet::isValid(address);
                break;
            case ChainType::ETHEREUM:
                valid = EthereumWallet::isValid(address);
                break;
            case ChainType::SOLANA:
                valid = SolanaWallet::isValid(address);
                break;
            default:
                break;
            }

            return valid;
        };

        if (!isValidAddress()) {
            MessageForm { this, 16, NO_VALID_ADDRESS }.exec();
            return;
        }

        addressRecord_->name = name;
        addressRecord_->address = address;
        addressRecord_->description = desc;
        addressRepo->update(*addressRecord_);
        accept();
    } else {
        text_->setChainType(type);

        auto addresses = text_->getValidAddresses();

        if (addresses.isEmpty()) {
            MessageForm { this, 5, NO_VALID_ADDRESSES }.exec();
            return;
        }

        try {

            Smart::AddressManager* manager = address_.manager;
            auto oldAddresses = manager->getAddresses();

            bool success
                = globalManager_->settingManager()
                      ->database()
                      ->storage()
                      ->transaction([&]() {
                          for (const QString& str : addresses) {
                              std::string addrValue = str.toStdString();

                              auto addrIt = std::find_if(oldAddresses.begin(),
                                  oldAddresses.end(),
                                  [&addrValue](const Address& addr) {
                                      return addr.address == addrValue;
                                  });

                              if (addrIt == oldAddresses.end()) {
                                  const std::string addressName
                                      = std::format("Address");
                                  const auto addressNameHash
                                      = Encryption::easyHash(addressName);
                                  const auto addressHash
                                      = Encryption::easyHash(addrValue);

                                  Address newAddr {
                                      .walletId = address_.walletId,
                                      .hash = Encryption::genRandomHash(),
                                      .name = addressName,
                                      .nameHash = addressNameHash,
                                      .address = addrValue,
                                      .addressHash = addressHash,
                                  };

                                  const auto id = addressRepo->insert(newAddr);
                                  if (id <= 0) {
                                      return false;
                                  }

                                  newAddr.id = id;

                                  addresses_.push_back(newAddr);
                                  manager->addAddress(newAddr);
                              }
                          }

                          return true;
                      });

            if (!success) {
                MessageForm { this, 16, "Failed to create addresses" }.exec();
                return;
            }

            accept();
        } catch (const std::exception& e) {
            std::cerr << "Failed to import addresses: " << e.what()
                      << std::endl;
            MessageForm { this, 5, NO_VALID_ADDRESSES }.exec();
            return;
        }
    }
}
