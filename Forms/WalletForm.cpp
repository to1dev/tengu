#include "WalletForm.h"
#include "ui_WalletForm.h"

WalletForm::WalletForm(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::WalletForm)
{
    ui->setupUi(this);
}

WalletForm::~WalletForm()
{
    delete ui;
}
