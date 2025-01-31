#include "Tengu.h"
#include "ui_Tengu.h"

Tengu::Tengu(
    const std::shared_ptr<const GlobalManager>& globalManager, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::Tengu)
    , mGlobalManager(globalManager)
{
    ui->setupUi(this);
    setWindowTitle(DEFAULT_TITLE);

    mFrameless = std::make_unique<Frameless>(this);
    mFrameless->setMainFrame(ui->frameMain);
    mFrameless->setContentFrame(ui->frameContent);
    mFrameless->setTopFrame(ui->topFrame);
    mFrameless->setMainMenu(ui->menubar);
    mFrameless->setButtonMin(ui->ButtonMin);
    mFrameless->setButtonMax(ui->ButtonMax);
    mFrameless->setButtonClose(ui->ButtonClose);
    mFrameless->init(true);

    mGlobalManager->getWindowManager()->setWindow(this);
    mGlobalManager->getWindowManager()->reset();

    ui->tabWidget->tabBar()->hide();

    initPopupMenu();
}

Tengu::~Tengu()
{
    delete ui;
}

void Tengu::onPopup()
{
    QPoint pt = ui->frameContent->mapToGlobal(ui->ButtonPopup->pos());
    mPopup->exec(pt);
}

void Tengu::reboot()
{
    QApplication::exit(Daitengu::Base::EXIT_CODE_REBOOT);
}

void Tengu::about()
{
}

void Tengu::initPopupMenu()
{
    mPopup = new QMenu(this);

    QMenu* appMenu = mPopup->addMenu(STR_MENU_APP);

    appMenu->addAction(STR_WINDOW_CENTER, this,
        [this]() { mGlobalManager->getWindowManager()->center(); });
    // appMenu->addAction(STR_APP_SOCKET, this, &Tengu::socket);
    appMenu->addSeparator();
    QAction* resetDbAction = appMenu->addAction(STR_APP_RESET_DB,
        [this]() { mGlobalManager->getSettingManager()->database()->reset(); });
    resetDbAction->setEnabled(false);

    appMenu->addSeparator();
    appMenu->addAction(STR_APP_REBOOT, this, &Tengu::reboot);

    mPopup->addAction(STR_MENU_ABOUT, this, &Tengu::about);
    mPopup->addSeparator();
    mPopup->addAction(STR_MENU_EXIT, this, &Tengu::close);
    mPopup->addSeparator();

    connect(ui->ButtonPopup, &QToolButton::released, this, &Tengu::onPopup);
}
