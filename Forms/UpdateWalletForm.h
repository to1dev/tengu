#pragma once

#include <QDialog>

namespace Ui {
class UpdateWalletForm;
}

class UpdateWalletForm : public QDialog {
    Q_OBJECT

public:
    explicit UpdateWalletForm(QWidget* parent = nullptr);
    ~UpdateWalletForm();

private:
    Ui::UpdateWalletForm* ui;
};
