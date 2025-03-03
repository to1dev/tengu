#include "Tengu.h"
#include "ui_Tengu.h"

Tengu::Tengu(
    const std::shared_ptr<const GlobalManager>& globalManager, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::Tengu)
    , globalManager_(globalManager)
{
    ui->setupUi(this);
    setWindowTitle(DEFAULT_TITLE);

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setTopFrame(ui->topFrame);
    frameless_->setMainMenu(ui->menubar);
    frameless_->init();

    // ui->tabWidget->tabBar()->hide();

    initPopupMenu();

    globalManager_->windowManager()->addWindow(
        WindowManager::WindowShape::TOPBAR, this);
    globalManager_->windowManager()->reset(
        this, 1, WindowManager::WindowShape::TOPBAR);

    connect(frameless_.get(), &Frameless::onMin, this, &Tengu::showMinimized);
    connect(frameless_.get(), &Frameless::onMax, this, [this]() {
        globalManager_->windowManager()->reset(
            this, 1, WindowManager::WindowShape::TOPBAR);
    });

    /*QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(QMargins(16, 16, 16, 16));
    layout->setSpacing(19);

    txList_ = new TxListWidget(this);
    txList_->setObjectName("listWidgetTask");

    walletPanel_ = new WalletPanel(this);
    walletPanel_->setObjectName("walletPanel");

    layout->addWidget(txList_);
    layout->addWidget(walletPanel_);
    ui->tabFirst->setLayout(layout);*/

    connect(ui->ButtonWallet, &QToolButton::clicked, this, &Tengu::wallet);
    connect(ui->action1, &QAction::triggered, this, &Tengu::showWalletDock);
}

Tengu::~Tengu()
{
    delete ui;
}

void Tengu::setWalletDock(const std::shared_ptr<WalletDock>& walletDock)
{
    walletDock_ = walletDock;
}

void Tengu::showWalletDock()
{
    walletDock_->show();
}

void Tengu::onPopup()
{
    // QPoint pt = ui->frameContent->mapToGlobal(ui->ButtonPopup->pos());
    // popup_->exec(pt);
}

void Tengu::reboot()
{
    QApplication::exit(Daitengu::Core::EXIT_CODE_REBOOT);
}

void Tengu::about()
{
    MessageForm mf(this, 100,
        "<strong>关于</strong> 这个程序！<p>Hello world!</p>", "About");
    mf.exec();
}

void Tengu::wallet()
{
    WalletForm wf(this, globalManager_);
    int ret = wf.exec();
    if (ret) {
    } else {
    }
}

void Tengu::initPopupMenu()
{
    popup_ = new QMenu(this);

    QMenu* appMenu = popup_->addMenu(STR_MENU_APP);

    appMenu->addAction(
        STR_WINDOW_CENTER, this, [this]() { windowManager_->center(); });
    // appMenu->addAction(STR_APP_SOCKET, this, &Tengu::socket);
    appMenu->addSeparator();
    QAction* resetDbAction = appMenu->addAction(STR_APP_RESET_DB,
        [this]() { globalManager_->settingManager()->database()->reset(); });
#ifndef USE_TEST
    resetDbAction->setEnabled(false);
#endif

    appMenu->addSeparator();
    appMenu->addAction(STR_APP_REBOOT, this, &Tengu::reboot);

    popup_->addAction(STR_MENU_ABOUT, this, &Tengu::about);
    popup_->addSeparator();
    popup_->addAction(STR_MENU_EXIT, this, &Tengu::close);
    popup_->addSeparator();

    // connect(ui->ButtonPopup, &QToolButton::released, this, &Tengu::onPopup);
}
