#include "WalletDock.h"
#include "ui_WalletDock.h"

WalletDock::WalletDock(
    QWidget* parent, const std::shared_ptr<const GlobalManager>& globalManager)
    : QWidget(parent)
    , ui(new Ui::WalletDock)
    , globalManager_(globalManager)
{
    ui->setupUi(this);

    setWindowTitle(DEFAULT_TITLE);

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->init(Frameless::Mode::PANEL);

    globalManager_->windowManager()->addWindow(
        WindowManager::WindowShape::RIGHT_PANEL, this);

    connect(frameless_.get(), &Frameless::onMax, this, [this]() {
        globalManager_->windowManager()->reset(
            this, 1, WindowManager::WindowShape::RIGHT_PANEL);
    });
}

WalletDock::~WalletDock()
{
    delete ui;
}
