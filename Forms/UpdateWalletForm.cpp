#include "UpdateWalletForm.h"
#include "ui_UpdateWalletForm.h"

UpdateWalletForm::UpdateWalletForm(
    QWidget* parent, const std::shared_ptr<const GlobalManager>& globalManager)
    : QDialog(parent)
    , ui(new Ui::UpdateWalletForm)
{
    ui->setupUi(this);

    setWindowTitle(DEFAULT_TITLE);

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->init();

    windowManager_->setWindow(this);
    windowManager_->reset(0.6);
}

UpdateWalletForm::~UpdateWalletForm()
{
    delete ui;
}
