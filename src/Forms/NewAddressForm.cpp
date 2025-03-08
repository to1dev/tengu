#include "NewAddressForm.h"
#include "ui_NewAddressForm.h"

NewAddressForm::NewAddressForm(
    QWidget* parent, const std::shared_ptr<const GlobalManager>& globalManager)
    : QDialog(parent)
    , ui(new Ui::NewAddressForm)
    , globalManager_(globalManager)
{
    ui->setupUi(this);

    setWindowTitle(DEFAULT_TITLE);

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->init(Frameless::Mode::DIALOG);

    globalManager_->windowManager()->reset(this, 0.6);
    connect(frameless_.get(), &Frameless::onMax, this,
        [this]() { globalManager_->windowManager()->reset(this, 0.6); });
}
