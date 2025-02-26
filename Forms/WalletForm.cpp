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
    connect(ui->ButtonDeleteWallet, &QPushButton::clicked, this,
        &WalletForm::delWallet);

    walletList_->load(
        globalManager_->settingManager()->database()->walletRepo()->getByGroup(
            static_cast<int>(WalletGroupType::User)));
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
        walletList_->add(*nwf.walletRecord());
    } else {
    }
}

void WalletForm::delWallet()
{
    /*QListWidgetItem* item = walletList_->currentItem();
    if (item && item->isSelected()) {
        int id = item->data(static_cast<int>(WalletListWidget::ItemData::id))
                     .toInt();
        MessageForm mf(this, CONFIRM_WALLET_DELETE, CONFIRM_WALLET_DELETE_TITLE,
            MessageButton::Ok | MessageButton::Cancel);
        int ret = mf.exec();
        if (ret) { }
    }*/

    if (auto* item = walletList_->currentItem(); item && item->isSelected()) {
        const auto id
            = item->data(static_cast<int>(WalletListWidget::ItemData::id))
                  .toInt();
        MessageForm mf(this, CONFIRM_WALLET_DELETE, CONFIRM_WALLET_DELETE_TITLE,
            MessageButton::Ok | MessageButton::Cancel);
        if (mf.exec()) {
            std::unique_ptr<QListWidgetItem> removedItem {
                walletList_->takeItem(walletList_->row(item))
            };
            walletList_->clearSelection();
            globalManager_->settingManager()->database()->walletRepo()->remove(
                id);
        }
    }
}
