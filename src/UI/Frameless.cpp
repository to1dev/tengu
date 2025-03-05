// SPDX-License-Identifier: AGPL-3.0-or-later
/*
 * Copyright (C) 2025 to1dev <https://arc20.me/to1dev>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "Frameless.h"

namespace Daitengu::UI {

Frameless::Frameless(QWidget* window)
    : window_(window)
{
}

void Frameless::init(const Mode& mode, bool fixed)
{
    if (!window_ || !mainFrame_)
        return;

    int index = 0;
    mode_ = mode;
    fixed_ = fixed;
    bool isMain = (mode == Mode::MAIN);
    Qt::WindowFlags flags;
    switch (mode) {
    case Mode::MAIN: {
        flags = Qt::FramelessWindowHint | Qt::Window
            | Qt::WindowMinMaxButtonsHint;
        break;
    }

    case Mode::DIALOG:
    case Mode::MESSAGEBOX: {
        flags = Qt::FramelessWindowHint | Qt::Dialog;
        break;
    }

    case Mode::PANEL: {
        flags = Qt::FramelessWindowHint | Qt::Tool;
        break;
    }

    default:
        break;
    }

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

    if (contentFrame_) {
        contentFrame_->setContentsMargins(QMargins(
            CONTENT_MARGIN, CONTENT_MARGIN, CONTENT_MARGIN, CONTENT_MARGIN));

        contentFrame_->setSizePolicy(
            QSizePolicy::Preferred, QSizePolicy::Expanding);

        if (contentFrame_->layout()) {
            contentFrame_->layout()->setSpacing(20);
        }
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
#ifdef v1
    QFrame* titleBar = new QFrame(mainFrame_);
#endif
    QWidget* titleBar = new QWidget(mainFrame_);
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
        if (!buttonMin_) {
            buttonMin_ = new QToolButton(bar);
            buttonMin_->setObjectName(STR_BUTTON_MIN);
            buttonMin_->setText(STR_BUTTON_MIN_TEXT);
        } else {
            layoutTitleBar->insertWidget(index++, buttonMin_);
            buttonMin_->setToolTip(STR_MAIN_TOOLTIP_MINIMIZE);
            connect(buttonMin_, &QToolButton::clicked, this, &Frameless::onMin);
        }

        if (!buttonMax_) {
            buttonMax_ = new QToolButton(bar);
            buttonMax_->setObjectName(STR_BUTTON_MAX);
            buttonMax_->setText(STR_BUTTON_MAX_TEXT);
        } else {
            layoutTitleBar->insertWidget(index++, buttonMax_);
            buttonMax_->setToolTip(STR_FORM_TOOLTIP_MAX);
            connect(buttonMax_, &QToolButton::clicked, this, &Frameless::onMax);
            connect(
                bar, &TitleBar::doubleClick, buttonMax_, &QToolButton::click);
        }

        if (buttonFixed_) {
            buttonFixed_->setCheckable(true);
            layoutTitleBar->insertWidget(index++, buttonFixed_);
            buttonFixed_->setToolTip(STR_FORM_TOOLTIP_FIXED);
            connect(
                buttonFixed_, &QToolButton::toggled, [this, bar](bool checked) {
                    fixed_ = checked;
                    bar->setFixed(fixed_);
                });
        }
        if (!buttonClose_) {
            buttonClose_ = new QToolButton(bar);
            buttonClose_->setObjectName(STR_BUTTON_CLOSE);
            buttonClose_->setText(STR_BUTTON_CLOSE_TEXT);
        } else {
            layoutTitleBar->insertWidget(index++, buttonClose_);
            buttonClose_->setToolTip(STR_MAIN_TOOLTIP_CLOSE);
            connect(buttonClose_, &QToolButton::clicked,
                [this]() { window_->close(); });
        }
    } else {
        if (mode_ != Mode::MESSAGEBOX && !buttonMax_) {
            buttonMax_ = new QToolButton(bar);
            buttonMax_->setObjectName(STR_BUTTON_MAX);
            buttonMax_->setText(STR_BUTTON_MAX_TEXT);
        }
        if (buttonMax_) {
            layoutTitleBar->insertWidget(index++, buttonMax_);
            buttonMax_->setToolTip(STR_FORM_TOOLTIP_MAX);
            connect(buttonMax_, &QToolButton::clicked, this, &Frameless::onMax);
            connect(
                bar, &TitleBar::doubleClick, buttonMax_, &QToolButton::click);
        }

        if (buttonFixed_) {
            buttonFixed_->setCheckable(true);
            layoutTitleBar->insertWidget(index++, buttonFixed_);

            buttonFixed_->setToolTip(STR_FORM_TOOLTIP_FIXED);

            fixed_ = buttonFixed_->isChecked();
            bar->setFixed(fixed_);

            connect(
                buttonFixed_, &QToolButton::toggled, [this, bar](bool checked) {
                    fixed_ = checked;
                    bar->setFixed(fixed_);
                });
        }

        connect(bar, &TitleBar::doubleClick, this, &Frameless::onMax);

        if (!buttonClose_) {
            buttonClose_ = new QToolButton(bar);
            buttonClose_->setText(STR_BUTTON_CLOSE_TEXT);
            buttonClose_->setObjectName(STR_BUTTON_CLOSE);
        } else {
            layoutTitleBar->insertWidget(index++, buttonClose_);
            buttonClose_->setToolTip(STR_FORM_TOOLTIP_CLOSE);
            connect(
                buttonClose_, &QToolButton::clicked, window_, &QWidget::close);
        }
    }

    QHBoxLayout* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(QMargins(0, 0, 0, 0));
    layout->setSpacing(0);

    index = 0;
#ifdef label_as_icon
    QLabel* labelIcon = new QLabel(bar);
    labelIcon->setPixmap(QPixmap(STR_MAIN_ICON));
    labelIcon->setMargin(ICON_MARGIN);
    layout->insertWidget(index++, labelIcon);
#endif

    if (isMain) {
        SVGWidget* svgIcon = new SVGWidget(STR_MAIN_ICON, bar);
        svgIcon->setContentsMargins(3, 3, 3, 3);
        svgIcon->setImageSize(QSize(24, 24));
        layout->insertWidget(index++, svgIcon);
    }

    QLabel* labelTitle = new QLabel(bar);
    labelTitle->setAttribute(Qt::WA_TransparentForMouseEvents);
    labelTitle->setObjectName(STR_LABEL_TITLE);
    if (isMain) {
        labelTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    } else {
        labelTitle->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    }
    labelTitle->setText(window_->windowTitle());
    layout->insertWidget(index++, labelTitle);

#ifdef title_always_left
    layout->addStretch(1);
#endif

    QHBoxLayout* layoutMiddle = nullptr;
    if (contentFrame_) {
        index = 0;
        layoutMiddle = new QHBoxLayout;
        layoutMiddle->setContentsMargins(QMargins(20, 20, 20, 20));
        layoutMiddle->setSpacing(9);
        layoutMiddle->insertWidget(index++, contentFrame_);
    }

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
                QString(":/%1/%2")
                    .arg(QString::fromUtf8(RandomLogos[index].first.data(),
                        RandomLogos[index].first.size()))
                    .arg(range),
                topFrame_);
            svgLogo->setImageSize(QSize(LOGO_SIZE, LOGO_SIZE));
            topLayout->addWidget(svgLogo);
        }

        layoutMain->insertWidget(index++, topFrame_, 1);
    }

    if (layoutMiddle)
        layoutMain->insertLayout(index++, layoutMiddle, 1);
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

void Frameless::max()
{
    switch (mode_) {
    case Mode::MAIN: {
        int currentHeight = window_->height();

        QRect screenGeometry
            = QGuiApplication::primaryScreen()->availableGeometry();

        window_->setGeometry(screenGeometry.x(), screenGeometry.y(),
            screenGeometry.width(), currentHeight);

        break;
    }

    case Mode::DIALOG: {
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

        break;
    }

    default:
        break;
    }
}

}
