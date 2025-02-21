#ifndef NEWWALLETFORM_H
#define NEWWALLETFORM_H

#include <QDialog>

#include "Managers/GlobalManager.h"
#include "Managers/WindowManager.h"

#include "UI/Frameless.h"

using namespace Daitengu::Core;
using namespace Daitengu::UI;

namespace Ui {
class NewWalletForm;
}

class NewWalletForm : public QDialog {
    Q_OBJECT

    const QString DEFAULT_TITLE = QObject::tr("New Wallet");

public:
    explicit NewWalletForm(QWidget* parent = nullptr,
        const std::shared_ptr<const GlobalManager>& globalManager = nullptr);
    ~NewWalletForm();

private:
    Ui::NewWalletForm* ui;

    std::shared_ptr<const GlobalManager> globalManager_;
    std::unique_ptr<WindowManager> windowManager_ {
        std::make_unique<WindowManager>()
    };
    std::unique_ptr<Frameless> frameless_;
};

#endif // NEWWALLETFORM_H
