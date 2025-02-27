#include "UpdateWalletForm.h"
#include "ui_UpdateWalletForm.h"

UpdateWalletForm::UpdateWalletForm(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::UpdateWalletForm)
{
}

UpdateWalletForm::~UpdateWalletForm()
{
    delete ui;
}
