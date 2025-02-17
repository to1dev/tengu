#include "Frameless.h"

namespace Daitengu::UI {

Frameless::Frameless(QWidget* window)
    : window_(window)
{
}

void Frameless::init(const bool isMain)
{
    if (!window_ || !mainFrame_ || !contentFrame_)
        return;

    int index = 0;
    Qt::WindowFlags flags = isMain
        ? Qt::FramelessWindowHint | Qt::Window | Qt::WindowMinMaxButtonsHint
        : Qt::FramelessWindowHint | Qt::Dialog;

    window_->setWindowFlags(flags);
    window_->setAttribute(Qt::WA_TranslucentBackground);

    if (isMain) {
        QMainWindow* mainWindow = dynamic_cast<QMainWindow*>(window_);
        if (mainWindow) {
            mainWindow->setContentsMargins(
                QMargins(MAIN_MARGIN, MAIN_MARGIN, MAIN_MARGIN, MAIN_MARGIN));
            mainWindow->setCentralWidget(mainFrame_);
        }
    } else {
        window_->setContentsMargins(
            QMargins(MAIN_MARGIN, MAIN_MARGIN, MAIN_MARGIN, MAIN_MARGIN));
    }

    contentFrame_->setContentsMargins(QMargins(
        CONTENT_MARGIN, CONTENT_MARGIN, CONTENT_MARGIN, CONTENT_MARGIN));

    if (contentFrame_->layout()) {
        contentFrame_->layout()->setSpacing(20);
    }

    QGraphicsDropShadowEffect* windowShadow = new QGraphicsDropShadowEffect;
    windowShadow->setBlurRadius(18.0);
    windowShadow->setColor(QColor(0, 0, 0, 220));
    windowShadow->setOffset(0.0);
    mainFrame_->setGraphicsEffect(windowShadow);

    QVBoxLayout* layoutMain = new QVBoxLayout(mainFrame_);
    layoutMain->setContentsMargins(QMargins(0, 0, 0, 0));
    layoutMain->setSpacing(0);

    QHBoxLayout* layoutTop = new QHBoxLayout;
    QFrame* titleBar = new QFrame(mainFrame_);
    titleBar->setObjectName(STR_TITLE_BAR);
    titleBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    layoutTop->insertWidget(index++, titleBar);
    QHBoxLayout* layoutTitleBar = new QHBoxLayout(titleBar);
    layoutTitleBar->setContentsMargins(QMargins(10, 10, 10, 10));
    layoutTitleBar->setSpacing(3);

    index = 0;
    TitleBar* bar = new TitleBar(window_);
    bar->setObjectName(STR_DRAG_BAR);
    bar->setFixed(fixed_);
    // bar->setAttribute(Qt::WA_OpaquePaintEvent);
    layoutTitleBar->insertWidget(index++, bar);

    if (isMain) {
        if (buttonMin_) {
            layoutTitleBar->insertWidget(index++, buttonMin_);
            buttonMin_->setToolTip(STR_MAIN_TOOLTIP_MINIMIZE);
            QObject::connect(
                buttonMin_, &QToolButton::clicked, this, &Frameless::min);
        }
        if (buttonMax_) {
            layoutTitleBar->insertWidget(index++, buttonMax_);
            buttonMax_->setToolTip(STR_FORM_TOOLTIP_MAX);
            QObject::connect(
                buttonMax_, &QToolButton::clicked, this, &Frameless::max);

            QObject::connect(
                bar, &TitleBar::doubleClick, buttonMax_, &QToolButton::click);
        }
        if (buttonFixed_) {
            buttonFixed_->setCheckable(true);
            layoutTitleBar->insertWidget(index++, buttonFixed_);
            buttonFixed_->setToolTip(STR_FORM_TOOLTIP_FIXED);
            QObject::connect(
                buttonFixed_, &QToolButton::toggled, [this, bar](bool checked) {
                    fixed_ = checked;
                    bar->setFixed(fixed_);
                });
        }
        if (buttonClose_) {
            layoutTitleBar->insertWidget(index++, buttonClose_);
            buttonClose_->setToolTip(STR_MAIN_TOOLTIP_CLOSE);
            QObject::connect(buttonClose_, &QToolButton::clicked,
                [this]() { window_->close(); });
        }
    } else {
        if (buttonFixed_) {
            buttonFixed_->setCheckable(true);
            layoutTitleBar->insertWidget(index++, buttonFixed_);

            buttonFixed_->setToolTip(STR_FORM_TOOLTIP_FIXED);

            fixed_ = buttonFixed_->isChecked();
            bar->setFixed(fixed_);

            QObject::connect(
                buttonFixed_, &QToolButton::toggled, [this, bar](bool checked) {
                    fixed_ = checked;
                    bar->setFixed(fixed_);
                });
        }
        if (buttonClose_) {
            layoutTitleBar->insertWidget(index++, buttonClose_);

            buttonClose_->setToolTip(STR_FORM_TOOLTIP_CLOSE);

            QObject::connect(
                buttonClose_, &QToolButton::clicked, window_, &QWidget::close);
        }
    }

    QHBoxLayout* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(QMargins(0, 0, 0, 0));
    layout->setSpacing(0);

    index = 0;
    /*QLabel* labelIcon = new QLabel(bar);
    labelIcon->setPixmap(QPixmap(STR_MAIN_ICON));
    labelIcon->setMargin(ICON_MARGIN);
    layout->insertWidget(index++, labelIcon);*/

    SVGWidget* svgIcon = new SVGWidget(STR_MAIN_ICON, bar);
    svgIcon->setContentsMargins(3, 3, 3, 3);
    svgIcon->setFixedSize(24, 24);
    layout->insertWidget(index++, svgIcon);

    QLabel* labelTitle = new QLabel(bar);
    labelTitle->setAttribute(Qt::WA_TransparentForMouseEvents);
    labelTitle->setObjectName(STR_LABEL_TITLE);
    if (isMain)
        labelTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    else
        labelTitle->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    labelTitle->setText(window_->windowTitle());
    layout->insertWidget(index++, labelTitle);
    layout->addStretch(1);

    index = 0;
    QHBoxLayout* layoutMiddle = new QHBoxLayout;
    layoutMiddle->setContentsMargins(QMargins(20, 20, 20, 20));
    layoutMiddle->setSpacing(9);
    layoutMiddle->insertWidget(index++, contentFrame_);

    index = 0;
    layoutMain->insertLayout(index++, layoutTop);
    if (mainMenu_) {
        layoutMain->insertWidget(index++, mainMenu_);
    }

    if (topFrame_) {
        topFrame_->setContentsMargins(QMargins(9, 9, 9, 9));

        if (topFrame_->layout()) {
            QBoxLayout* topLayout
                = qobject_cast<QBoxLayout*>(topFrame_->layout());
            topLayout->addStretch(1);

            int index = randomIndex(0, RandomLogos.size() - 1);
            int range = randomIndex(
                RandomLogos[index].second.start, RandomLogos[index].second.end);

            /*QPixmap logoPixmap(
                QString(":/%1/%2").arg(RandomLogos[index].first).arg(range));
            QLabel* labelLogo = new QLabel(mTopFrame);
            labelLogo->setMargin(3);
            labelLogo->setPixmap(logoPixmap.scaled(LOGO_SIZE, LOGO_SIZE,
                Qt::KeepAspectRatio, Qt::SmoothTransformation));
            topLayout->addWidget(labelLogo);*/

            SVGWidget* svgLogo = new SVGWidget(
                QString(":/%1/%2").arg(RandomLogos[index].first).arg(range),
                topFrame_);
            svgLogo->setFixedSize(LOGO_SIZE, LOGO_SIZE);
            topLayout->addWidget(svgLogo);
        }

        layoutMain->insertWidget(index++, topFrame_);
        layoutMain->insertLayout(index++, layoutMiddle);
    } else
        layoutMain->insertLayout(index++, layoutMiddle);
}

void Frameless::setMainFrame(QWidget* newMainFrame)
{
    mainFrame_ = newMainFrame;
}

void Frameless::setTopFrame(QWidget* newTopFrame)
{
    topFrame_ = newTopFrame;
}

void Frameless::setContentFrame(QWidget* newContentFrame)
{
    contentFrame_ = newContentFrame;
}

void Frameless::setButtonMin(QToolButton* newButtonMin)
{
    buttonMin_ = newButtonMin;
}

void Frameless::setButtonMax(QToolButton* newButtonMax)
{
    buttonMax_ = newButtonMax;
}

void Frameless::setButtonClose(QToolButton* newButtonClose)
{
    buttonClose_ = newButtonClose;
}

void Frameless::setButtonFixed(QToolButton* newButtonFixed)
{
    buttonFixed_ = newButtonFixed;
}

void Frameless::setMainMenu(QMenuBar* newMainMenu)
{
    mainMenu_ = newMainMenu;
}

void Frameless::min()
{
    window_->showMinimized();
}

void Frameless::max()
{
    if (window_->isMaximized()) {
        buttonMax_->setToolTip(STR_FORM_TOOLTIP_MAX);
        window_->setContentsMargins(
            QMargins(MAIN_MARGIN, MAIN_MARGIN, MAIN_MARGIN, MAIN_MARGIN));
        mainFrame_->setStyleSheet("#frameMain {border-radius: 6;}");
        window_->showNormal();
    } else {
        buttonMax_->setToolTip(STR_FORM_TOOLTIP_NORMAL);
        window_->setContentsMargins(0, 0, 0, 0);
        mainFrame_->setStyleSheet("#frameMain {border-radius: 0;}");
        window_->showMaximized();
    }
}

bool Frameless::fixed() const
{
    return fixed_;
}

void Frameless::setFixed(bool newFixed)
{
    fixed_ = newFixed;
}

}
