#include "WalletForm.h"
#include "ui_WalletForm.h"

WalletForm::WalletForm(
    QWidget* parent, const std::shared_ptr<const GlobalManager>& globalManager)
    : QDialog(parent)
    , ui(new Ui::WalletForm)
    , globalManager_(globalManager)
{
    ui->setupUi(this);

    setWindowTitle(DEFAULT_TITLE);

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->setButtonClose(ui->ButtonClose);
    frameless_->init();

    QHBoxLayout* layoutPanel = new QHBoxLayout(ui->groupBoxWallets);
    layoutPanel->setContentsMargins(DEFAULT_GROUP_MARGINS);

    walletList_ = new WalletListWidget(this);

    QWidget* panelButtons = new QWidget(this);
    panelButtons->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    panelButtons->setContentsMargins(QMargins(0, 0, 0, 0));

    QVBoxLayout* layoutButtons = new QVBoxLayout(panelButtons);
    layoutButtons->setContentsMargins(QMargins(20, 0, 0, 0));
    layoutButtons->setSpacing(DEFAULT_SPACING);
    layoutButtons->addWidget(ui->ButtonNewWallet);
    layoutButtons->addWidget(ui->ButtonImportWallet);
    layoutButtons->addWidget(ui->ButtonEditWallet);
    layoutButtons->addWidget(ui->ButtonDeleteWallet);
    layoutButtons->addStretch(1);
    panelButtons->setLayout(layoutButtons);

    layoutPanel->addWidget(walletList_);
    layoutPanel->addWidget(panelButtons);
    ui->groupBoxWallets->setLayout(layoutPanel);

    windowManager_->setWindow(this);
    windowManager_->reset(0.7);

    connect(ui->ButtonOK, &QPushButton::clicked, this, &WalletForm::ok);
    connect(ui->ButtonNewWallet, &QPushButton::clicked, this,
        &WalletForm::newWallet);
}

WalletForm::~WalletForm()
{
    delete ui;
}

void WalletForm::ok()
{
    accept();
}

void WalletForm::newWallet()
{
    NewWalletForm nwf(this, globalManager_);
    int ret = nwf.exec();
    if (ret) {
    } else {
    }
}
