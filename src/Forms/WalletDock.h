#pragma once

#include <QWidget>

#include "Managers/GlobalManager.h"
#include "Managers/WindowManager.h"

#include "UI/Frameless.h"

using namespace Daitengu::Core;
using namespace Daitengu::UI;

namespace Ui {
class WalletDock;
}

class WalletDock : public QWidget {
    Q_OBJECT

    const QString DEFAULT_TITLE = QObject::tr("My Wallet");

public:
    explicit WalletDock(QWidget* parent = nullptr,
        const std::shared_ptr<const GlobalManager>& globalManager = nullptr);
    ~WalletDock();

private:
    Ui::WalletDock* ui;

    std::shared_ptr<const GlobalManager> globalManager_;
    std::unique_ptr<Frameless> frameless_;
};
