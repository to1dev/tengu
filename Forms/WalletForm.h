#ifndef WALLETFORM_H
#define WALLETFORM_H

#include <QDialog>

#include "Managers/GlobalManager.h"

#include "UI/Frameless.h"

using namespace Daitengu::Core;
using namespace Daitengu::UI;

namespace Ui {
class WalletForm;
}

class WalletForm : public QDialog {
    Q_OBJECT

    const QString DEFAULT_TITLE = QObject::tr("Wallets");

public:
    explicit WalletForm(QWidget* parent = nullptr,
        const std::shared_ptr<const GlobalManager>& globalManager = nullptr);
    ~WalletForm();

private:
    Ui::WalletForm* ui;

    std::shared_ptr<const GlobalManager> globalManager_;
    std::unique_ptr<Frameless> frameless_;
};

#endif // WALLETFORM_H
