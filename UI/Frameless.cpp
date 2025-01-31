#include "Frameless.h"

namespace Daitengu::UI {

Frameless::Frameless(QWidget* window)
    : mWindow(window)
{
}

void Frameless::init(const bool isMain)
{
    if (!mWindow || !mMainFrame || !mContentFrame)
        return;

    int index = 0;
    Qt::WindowFlags flags = isMain
        ? Qt::FramelessWindowHint | Qt::Window | Qt::WindowMinMaxButtonsHint
        : Qt::FramelessWindowHint | Qt::Dialog;

    mWindow->setWindowFlags(flags);
    mWindow->setAttribute(Qt::WA_TranslucentBackground);

    if (isMain) {
        QMainWindow* mainWindow = dynamic_cast<QMainWindow*>(mWindow);
        if (mainWindow) {
            mainWindow->setContentsMargins(
                QMargins(MAIN_MARGIN, MAIN_MARGIN, MAIN_MARGIN, MAIN_MARGIN));
            mainWindow->setCentralWidget(mMainFrame);
        }
    } else {
        mWindow->setContentsMargins(
            QMargins(MAIN_MARGIN, MAIN_MARGIN, MAIN_MARGIN, MAIN_MARGIN));
    }

    mContentFrame->setContentsMargins(QMargins(
        CONTENT_MARGIN, CONTENT_MARGIN, CONTENT_MARGIN, CONTENT_MARGIN));

    if (mContentFrame->layout()) {
        mContentFrame->layout()->setSpacing(20);
    }

    QGraphicsDropShadowEffect* windowShadow = new QGraphicsDropShadowEffect;
    windowShadow->setBlurRadius(18.0);
    windowShadow->setColor(QColor(0, 0, 0, 220));
    windowShadow->setOffset(0.0);
    mMainFrame->setGraphicsEffect(windowShadow);

    QVBoxLayout* layoutMain = new QVBoxLayout(mMainFrame);
    layoutMain->setContentsMargins(QMargins(0, 0, 0, 0));
    layoutMain->setSpacing(0);

    QHBoxLayout* layoutTop = new QHBoxLayout;
    QFrame* titleBar = new QFrame(mMainFrame);
    titleBar->setObjectName(STR_TITLE_BAR);
    titleBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    layoutTop->insertWidget(index++, titleBar);
    QHBoxLayout* layoutTitleBar = new QHBoxLayout(titleBar);
    layoutTitleBar->setContentsMargins(QMargins(10, 10, 10, 10));
    layoutTitleBar->setSpacing(3);

    index = 0;
    TitleBar* bar = new TitleBar(mWindow);
    bar->setObjectName(STR_DRAG_BAR);
    bar->setFixed(mFixed);
    // bar->setAttribute(Qt::WA_OpaquePaintEvent);
    layoutTitleBar->insertWidget(index++, bar);

    if (isMain) {
        if (mButtonMin) {
            layoutTitleBar->insertWidget(index++, mButtonMin);
            mButtonMin->setToolTip(STR_MAIN_TOOLTIP_MINIMIZE);
            QObject::connect(
                mButtonMin, &QToolButton::clicked, this, &Frameless::min);
        }
        if (mButtonMax) {
            layoutTitleBar->insertWidget(index++, mButtonMax);
            mButtonMax->setToolTip(STR_FORM_TOOLTIP_MAX);
            QObject::connect(
                mButtonMax, &QToolButton::clicked, this, &Frameless::max);

            QObject::connect(
                bar, &TitleBar::doubleClick, mButtonMax, &QToolButton::click);
        }
        if (mButtonFixed) {
            mButtonFixed->setCheckable(true);
            layoutTitleBar->insertWidget(index++, mButtonFixed);
            mButtonFixed->setToolTip(STR_FORM_TOOLTIP_FIXED);
            QObject::connect(
                mButtonFixed, &QToolButton::toggled, [this, bar](bool checked) {
                    mFixed = checked;
                    bar->setFixed(mFixed);
                });
        }
        if (mButtonClose) {
            layoutTitleBar->insertWidget(index++, mButtonClose);
            mButtonClose->setToolTip(STR_MAIN_TOOLTIP_CLOSE);
            QObject::connect(mButtonClose, &QToolButton::clicked,
                [this]() { mWindow->close(); });
        }
    } else {
        if (mButtonFixed) {
            mButtonFixed->setCheckable(true);
            layoutTitleBar->insertWidget(index++, mButtonFixed);

            mButtonFixed->setToolTip(STR_FORM_TOOLTIP_FIXED);

            mFixed = mButtonFixed->isChecked();
            bar->setFixed(mFixed);

            QObject::connect(
                mButtonFixed, &QToolButton::toggled, [this, bar](bool checked) {
                    mFixed = checked;
                    bar->setFixed(mFixed);
                });
        }
        if (mButtonClose) {
            layoutTitleBar->insertWidget(index++, mButtonClose);

            mButtonClose->setToolTip(STR_FORM_TOOLTIP_CLOSE);

            QObject::connect(
                mButtonClose, &QToolButton::clicked, mWindow, &QWidget::close);
        }
    }

    QHBoxLayout* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(QMargins(0, 0, 0, 0));
    layout->setSpacing(0);

    index = 0;
    QLabel* labelIcon = new QLabel(bar);
    labelIcon->setPixmap(QPixmap(STR_MAIN_ICON));
    labelIcon->setMargin(ICON_MARGIN);
    layout->insertWidget(index++, labelIcon);

    QLabel* labelTitle = new QLabel(bar);
    labelTitle->setAttribute(Qt::WA_TransparentForMouseEvents);
    labelTitle->setObjectName(STR_LABEL_TITLE);
    if (isMain)
        labelTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    else
        labelTitle->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    labelTitle->setText(mWindow->windowTitle());
    layout->insertWidget(index++, labelTitle);
    layout->addStretch(1);

    index = 0;
    QHBoxLayout* layoutMiddle = new QHBoxLayout;
    layoutMiddle->setContentsMargins(QMargins(20, 20, 20, 20));
    layoutMiddle->setSpacing(9);
    layoutMiddle->insertWidget(index++, mContentFrame);

    index = 0;
    layoutMain->insertLayout(index++, layoutTop);
    if (mMainMenu) {
        layoutMain->insertWidget(index++, mMainMenu);
    }

    if (mTopFrame) {
        mTopFrame->setContentsMargins(QMargins(9, 9, 9, 9));

        if (mTopFrame->layout()) {
            QBoxLayout* topLayout
                = qobject_cast<QBoxLayout*>(mTopFrame->layout());
            topLayout->addStretch(1);

            int index = randomIndex(0, RandomLogos.size() - 1);
            int range = randomIndex(
                RandomLogos[index].second.start, RandomLogos[index].second.end);
            QPixmap logoPixmap(
                QString(":/%1/%2").arg(RandomLogos[index].first).arg(range));

            QLabel* labelLogo = new QLabel(mTopFrame);
            labelLogo->setMargin(3);
            labelLogo->setPixmap(logoPixmap.scaled(LOGO_SIZE, LOGO_SIZE,
                Qt::KeepAspectRatio, Qt::SmoothTransformation));
            topLayout->addWidget(labelLogo);
        }

        layoutMain->insertWidget(index++, mTopFrame);
        layoutMain->insertLayout(index++, layoutMiddle);
    } else
        layoutMain->insertLayout(index++, layoutMiddle);
}

void Frameless::setMainFrame(QWidget* newMainFrame)
{
    mMainFrame = newMainFrame;
}

void Frameless::setTopFrame(QWidget* newTopFrame)
{
    mTopFrame = newTopFrame;
}

void Frameless::setContentFrame(QWidget* newContentFrame)
{
    mContentFrame = newContentFrame;
}

void Frameless::setButtonMin(QToolButton* newButtonMin)
{
    mButtonMin = newButtonMin;
}

void Frameless::setButtonMax(QToolButton* newButtonMax)
{
    mButtonMax = newButtonMax;
}

void Frameless::setButtonClose(QToolButton* newButtonClose)
{
    mButtonClose = newButtonClose;
}

void Frameless::setButtonFixed(QToolButton* newButtonFixed)
{
    mButtonFixed = newButtonFixed;
}

void Frameless::setMainMenu(QMenuBar* newMainMenu)
{
    mMainMenu = newMainMenu;
}

void Frameless::min()
{
    mWindow->showMinimized();
}

void Frameless::max()
{
    if (mWindow->isMaximized()) {
        mButtonMax->setToolTip(STR_FORM_TOOLTIP_MAX);
        mWindow->setContentsMargins(
            QMargins(MAIN_MARGIN, MAIN_MARGIN, MAIN_MARGIN, MAIN_MARGIN));
        mMainFrame->setStyleSheet("#frameMain {border-radius: 6;}");
        mWindow->showNormal();
    } else {
        mButtonMax->setToolTip(STR_FORM_TOOLTIP_NORMAL);
        mWindow->setContentsMargins(0, 0, 0, 0);
        mMainFrame->setStyleSheet("#frameMain {border-radius: 0;}");
        mWindow->showMaximized();
    }
}

bool Frameless::fixed() const
{
    return mFixed;
}

void Frameless::setFixed(bool newFixed)
{
    mFixed = newFixed;
}

}
