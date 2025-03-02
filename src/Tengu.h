#ifndef TENGU_H
#define TENGU_H

#include <QHBoxLayout>
#include <QMainWindow>
#include <QTabBar>

#include "Consts.h"
#include "Globals.h"

#include "Managers/GlobalManager.h"
#include "Managers/WindowManager.h"

#include "UI/Frameless.h"

#include "Components/TxListWidget.h"
#include "Components/WalletPanel.h"

#include "Forms/MessageForm.h"
#include "Forms/WalletDock.h"
#include "Forms/WalletForm.h"

using namespace Daitengu::Components;
using namespace Daitengu::Core;
using namespace Daitengu::UI;

QT_BEGIN_NAMESPACE

namespace Ui {
class Tengu;
}

QT_END_NAMESPACE

class Tengu : public QMainWindow {
    Q_OBJECT

    const QString DEFAULT_TITLE = QObject::tr("Tengu");

public:
    Tengu(const std::shared_ptr<const GlobalManager>& globalManager,
        QWidget* parent = nullptr);
    ~Tengu();

private Q_SLOTS:
    void onShowToolWindow();

    void onPopup();
    void reboot();
    void about();
    void wallet();

private:
    void initPopupMenu();

private:
    Ui::Tengu* ui;

    std::unique_ptr<WalletDock> toolWindow;

    std::shared_ptr<const GlobalManager> globalManager_;
    std::unique_ptr<WindowManager> windowManager_ {
        std::make_unique<WindowManager>()
    };
    std::unique_ptr<Frameless> frameless_;

    QMenu* popup_;

    WalletPanel* walletPanel_;
    TxListWidget* txList_;
};
#endif // TENGU_H
