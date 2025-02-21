#ifndef WALLETFORM_H
#define WALLETFORM_H

#include <QDialog>

namespace Ui {
class WalletForm;
}

class WalletForm : public QDialog {
    Q_OBJECT

public:
    explicit WalletForm(QWidget* parent = nullptr);
    ~WalletForm();

private:
    Ui::WalletForm* ui;
};

#endif // WALLETFORM_H
