#ifndef WALLETFORM_H
#define WALLETFORM_H

#include <QDialog>

#include "Consts.h"

#include "Managers/GlobalManager.h"
#include "Managers/WindowManager.h"

#include "Components/WalletListWidget.h"

#include "UI/Frameless.h"

#include "Forms/NewWalletForm.h"

using namespace Daitengu::Components;
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

private Q_SLOTS:
    void ok();

private:
    void newWallet();
    void delWallet();

private:
    Ui::WalletForm* ui;

    std::shared_ptr<const GlobalManager> globalManager_;
    std::unique_ptr<WindowManager> windowManager_ {
        std::make_unique<WindowManager>()
    };
    std::unique_ptr<Frameless> frameless_;

    WalletListWidget* walletList_;
};

#endif // WALLETFORM_H
