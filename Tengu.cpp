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

    globalManager_->layoutManager()->setWindow(this);
    globalManager_->layoutManager()->reset();

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
    popup_->exec(pt);
}

void Tengu::reboot()
{
    QApplication::exit(Daitengu::Core::EXIT_CODE_REBOOT);
}

void Tengu::about()
{
}

void Tengu::initPopupMenu()
{
    popup_ = new QMenu(this);

    QMenu* appMenu = popup_->addMenu(STR_MENU_APP);

    appMenu->addAction(STR_WINDOW_CENTER, this,
        [this]() { globalManager_->layoutManager()->center(); });
    // appMenu->addAction(STR_APP_SOCKET, this, &Tengu::socket);
    appMenu->addSeparator();
    QAction* resetDbAction = appMenu->addAction(STR_APP_RESET_DB,
        [this]() { globalManager_->settingManager()->database()->reset(); });
    resetDbAction->setEnabled(false);

    appMenu->addSeparator();
    appMenu->addAction(STR_APP_REBOOT, this, &Tengu::reboot);

    popup_->addAction(STR_MENU_ABOUT, this, &Tengu::about);
    popup_->addSeparator();
    popup_->addAction(STR_MENU_EXIT, this, &Tengu::close);
    popup_->addSeparator();

    connect(ui->ButtonPopup, &QToolButton::released, this, &Tengu::onPopup);
}
