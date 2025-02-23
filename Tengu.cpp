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
    frameless_->setContentFrame(ui->frameContent);
    frameless_->setTopFrame(ui->topFrame);
    frameless_->setMainMenu(ui->menubar);
    frameless_->setButtonMin(ui->ButtonMin);
    frameless_->setButtonMax(ui->ButtonMax);
    frameless_->setButtonClose(ui->ButtonClose);
    frameless_->init(true);

    windowManager_->setWindow(this);
    windowManager_->reset(0.9);

    ui->tabWidget->tabBar()->hide();

    initPopupMenu();

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(QMargins(16, 16, 16, 16));
    layout->setSpacing(19);

    txList_ = new TxListWidget(this);
    txList_->setObjectName("listWidgetTask");

    walletPanel_ = new WalletPanel(this);
    walletPanel_->setObjectName("walletPanel");

    layout->addWidget(txList_);
    layout->addWidget(walletPanel_);
    ui->tabFirst->setLayout(layout);

    connect(ui->ButtonWallet, &QToolButton::clicked, this, &Tengu::wallet);
}

Tengu::~Tengu()
{
    delete ui;
}

void Tengu::onPopup()
{
    QPoint pt = ui->frameContent->mapToGlobal(ui->ButtonPopup->pos());
    popup_->exec(pt);
}

void Tengu::reboot()
{
    QApplication::exit(Daitengu::Core::EXIT_CODE_REBOOT);
}

void Tengu::about()
{
    MessageForm mf(this);
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
    resetDbAction->setEnabled(true);

    appMenu->addSeparator();
    appMenu->addAction(STR_APP_REBOOT, this, &Tengu::reboot);

    popup_->addAction(STR_MENU_ABOUT, this, &Tengu::about);
    popup_->addSeparator();
    popup_->addAction(STR_MENU_EXIT, this, &Tengu::close);
    popup_->addSeparator();

    connect(ui->ButtonPopup, &QToolButton::released, this, &Tengu::onPopup);
}
