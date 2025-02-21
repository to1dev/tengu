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

    globalManager_->windowManager()->setWindow(this);
    globalManager_->windowManager()->reset(0.6);
}

WalletForm::~WalletForm()
{
    delete ui;
}
