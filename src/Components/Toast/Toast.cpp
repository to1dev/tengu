// SPDX-License-Identifier: AGPL-3.0-or-later
/*
 * Copyright (C) 2024 Niklas Henning <https://github.com/niklashenning/qt-toast>
 * Copyright (C) 2024 to1dev <https://arc20.me/to1dev>
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

#include "Toast.h"

namespace Daitengu::Components {

constinit int ToastManager::maximumOnScreen_ = 3;
constinit int ToastManager::spacing_ = 10;
constinit int ToastManager::offsetX_ = 20;
constinit int ToastManager::offsetY_ = 45;
constinit bool ToastManager::alwaysOnMainScreen_ = false;
std::optional<QScreen*> ToastManager::fixedScreen_ = std::nullopt;
constinit ToastPosition ToastManager::position_ = ToastPosition::BOTTOM_RIGHT;
std::deque<Toast*> ToastManager::currentlyShown_;
std::deque<Toast*> ToastManager::queue_;
std::mutex ToastManager::mutex_;

const QColor Toast::successAccentColor_ = QColor("#3E9141");
const QColor Toast::warningAccentColor_ = QColor("#E8B849");
const QColor Toast::errorAccentColor_ = QColor("#BA2626");
const QColor Toast::informationAccentColor_ = QColor("#007FFF");
const QColor Toast::defaultAccentColor_ = QColor("#5C5C5C");
const QColor Toast::defaultBackgroundColor_ = QColor("#E7F4F9");
const QColor Toast::defaultTitleColor_ = QColor("#000000");
const QColor Toast::defaultTextColor_ = QColor("#5C5C5C");
const QColor Toast::defaultIconSeparatorColor_ = QColor("#D9D9D9");
const QColor Toast::defaultCloseButtonIconColor_ = QColor("#000000");
const QColor Toast::defaultBackgroundColorDark_ = QColor("#292929");
const QColor Toast::defaultTitleColorDark_ = QColor("#FFFFFF");
const QColor Toast::defaultTextColorDark_ = QColor("#D0D0D0");
const QColor Toast::defaultIconSeparatorColorDark_ = QColor("#585858");
const QColor Toast::defaultCloseButtonIconColorDark_ = QColor("#C9C9C9");

int ToastManager::getMaximumOnScreen()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return maximumOnScreen_;
}

int ToastManager::getSpacing()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return spacing_;
}

QPoint ToastManager::getOffset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return QPoint(offsetX_, offsetY_);
}

int ToastManager::getOffsetX()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return offsetX_;
}

int ToastManager::getOffsetY()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return offsetY_;
}

bool ToastManager::isAlwaysOnMainScreen()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return alwaysOnMainScreen_;
}

std::optional<std::reference_wrapper<QScreen>> ToastManager::getFixedScreen()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (fixedScreen_.has_value()) {
        return std::ref(*fixedScreen_.value());
    }
    return std::nullopt;
}

ToastPosition ToastManager::getPosition()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return position_;
}

int ToastManager::getCount()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return currentlyShown_.size() + queue_.size();
}

int ToastManager::getVisibleCount()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return currentlyShown_.size();
}

int ToastManager::getQueuedCount()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

void ToastManager::setMaximumOnScreen(int maximum)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int freedSpaces = maximum - maximumOnScreen_;
    maximumOnScreen_ = maximum;

    if (freedSpaces > 0) {
        for (int i = 0; i < freedSpaces && !queue_.empty(); i++) {
            showNextInQueue();
        }
    }
}

void ToastManager::setSpacing(int spacing)
{
    std::lock_guard<std::mutex> lock(mutex_);
    spacing_ = spacing;
    updateCurrentlyShowingPositionY();
}

void ToastManager::setOffset(int x, int y)
{
    std::lock_guard<std::mutex> lock(mutex_);
    offsetX_ = x;
    offsetY_ = y;
    updateCurrentlyShowingPositionXY();
}

void ToastManager::setOffsetX(int offsetX)
{
    std::lock_guard<std::mutex> lock(mutex_);
    offsetX_ = offsetX;
    updateCurrentlyShowingPositionX();
}

void ToastManager::setOffsetY(int offsetY)
{
    std::lock_guard<std::mutex> lock(mutex_);
    offsetY_ = offsetY;
    updateCurrentlyShowingPositionY();
}

void ToastManager::setAlwaysOnMainScreen(bool enabled)
{
    std::lock_guard<std::mutex> lock(mutex_);
    alwaysOnMainScreen_ = enabled;
    updateCurrentlyShowingPositionXY();
}

void ToastManager::setFixedScreen(QScreen* screen)
{
    std::lock_guard<std::mutex> lock(mutex_);
    fixedScreen_ = screen;
    updateCurrentlyShowingPositionXY();
}

void ToastManager::setPosition(ToastPosition position)
{
    std::lock_guard<std::mutex> lock(mutex_);
    position_ = position;
    updateCurrentlyShowingPositionXY();
}

void ToastManager::reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    maximumOnScreen_ = 3;
    spacing_ = 10;
    offsetX_ = 20;
    offsetY_ = 45;
    alwaysOnMainScreen_ = false;
    fixedScreen_ = std::nullopt;
    position_ = ToastPosition::BOTTOM_RIGHT;

    for (Toast* toast : currentlyShown_) {
        toast->setVisible(false);
        toast->deleteLater();
    }

    currentlyShown_.clear();
    queue_.clear();
}

void ToastManager::addToast(Toast* toast)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (maximumOnScreen_ > currentlyShown_.size()) {
        currentlyShown_.push_back(toast);
    } else {
        queue_.push_back(toast);
    }
}

void ToastManager::removeToast(Toast* toast)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find(currentlyShown_.begin(), currentlyShown_.end(), toast);
    if (it != currentlyShown_.end()) {
        currentlyShown_.erase(it);
    }
}

void ToastManager::showNextInQueue()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!queue_.empty() && currentlyShown_.size() < maximumOnScreen_) {
        Toast* nextToast = queue_.front();
        queue_.pop_front();
        QMetaObject::invokeMethod(nextToast, "show", Qt::QueuedConnection);
    }
}

void ToastManager::updateCurrentlyShowingPositionXY()
{
    for (auto toast : currentlyShown_) {
        toast->updatePositionXY();
    }
}

void ToastManager::updateCurrentlyShowingPositionX()
{
    for (auto toast : currentlyShown_) {
        toast->updatePositionX();
    }
}

void ToastManager::updateCurrentlyShowingPositionY()
{
    for (auto toast : currentlyShown_) {
        toast->updatePositionY();
    }
}

const QString Toast::TOAST_STYLE = R"(
#toast-drop-shadow-layer-1 {
    background: rgba(0, 0, 0, 3);
    border-radius: 8px;
}

#toast-drop-shadow-layer-2 {
    background: rgba(0, 0, 0, 5);
    border-radius: 8px;
}

#toast-drop-shadow-layer-3 {
    background: rgba(0, 0, 0, 6);
    border-radius: 8px;
}

#toast-drop-shadow-layer-4 {
    background: rgba(0, 0, 0, 9);
    border-radius: 8px;
}

#toast-drop-shadow-layer-5 {
    background: rgba(0, 0, 0, 10);
    border-radius: 8px;
}

#toast-close-button {
    background: transparent;
}

#toast-icon-widget {
    background: transparent;
}
)";

Toast::Toast(QWidget* parent)
    : QDialog(parent)
    , parent_(parent)
{
    backgroundColor_ = defaultBackgroundColor_;
    titleColor_ = defaultTitleColor_;
    textColor_ = defaultTextColor_;
    iconColor_ = defaultAccentColor_;
    iconSeparatorColor_ = defaultIconSeparatorColor_;
    closeButtonIconColor_ = defaultCloseButtonIconColor_;
    durationBarColor_ = defaultAccentColor_;
    titleFont_ = QFont("Arial", 9, QFont::Weight::Bold);
    textFont_ = QFont("Arial", 9);

    notification_ = std::make_unique<QLabel>(this);
    dropShadowLayer1_ = std::make_unique<QWidget>(this);
    dropShadowLayer2_ = std::make_unique<QWidget>(this);
    dropShadowLayer3_ = std::make_unique<QWidget>(this);
    dropShadowLayer4_ = std::make_unique<QWidget>(this);
    dropShadowLayer5_ = std::make_unique<QWidget>(this);
    opacityEffect_ = std::make_unique<QGraphicsOpacityEffect>();
    closeButton_ = std::make_unique<QPushButton>(notification_.get());
    titleLabel_ = std::make_unique<QLabel>(notification_.get());
    textLabel_ = std::make_unique<QLabel>(notification_.get());
    iconWidget_ = std::make_unique<QPushButton>(notification_.get());
    iconSeparator_ = std::make_unique<QWidget>(notification_.get());
    durationBarContainer_ = std::make_unique<QWidget>(notification_.get());
    durationBar_ = std::make_unique<QWidget>(durationBarContainer_.get());
    durationBarChunk_ = std::make_unique<QWidget>(durationBarContainer_.get());
    durationTimer_ = std::make_unique<QTimer>(this);
    durationBarTimer_ = std::make_unique<QTimer>(this);

    setAttribute(Qt::WidgetAttribute::WA_TranslucentBackground);
    setFocusPolicy(Qt::FocusPolicy::NoFocus);

    dropShadowLayer1_->setObjectName("toast-drop-shadow-layer-1");
    dropShadowLayer2_->setObjectName("toast-drop-shadow-layer-2");
    dropShadowLayer3_->setObjectName("toast-drop-shadow-layer-3");
    dropShadowLayer4_->setObjectName("toast-drop-shadow-layer-4");
    dropShadowLayer5_->setObjectName("toast-drop-shadow-layer-5");
    closeButton_->setObjectName("toast-close-button");
    iconWidget_->setObjectName("toast-icon-widget");

    opacityEffect_->setOpacity(1);
    setGraphicsEffect(opacityEffect_.get());

    closeButton_->setCursor(Qt::CursorShape::PointingHandCursor);
    connect(closeButton_.get(), &QPushButton::clicked, this, &Toast::hide);

    durationBarContainer_->setFixedHeight(4);
    durationBarContainer_->setStyleSheet("background: transparent;");

    durationBar_->setFixedHeight(20);
    durationBar_->move(0, -16);

    durationBarChunk_->setFixedHeight(20);
    durationBarChunk_->move(0, -16);

    durationTimer_->setSingleShot(true);
    connect(durationTimer_.get(), &QTimer::timeout, this, &Toast::fadeOut);
    connect(durationBarTimer_.get(), &QTimer::timeout, this,
        &Toast::updateDurationBar);

    icon_ = getIconFromEnum(ToastIcon::INFORMATION);
    closeButtonIcon_ = getIconFromEnum(ToastIcon::CLOSE);

    setIcon(icon_);
    setIconSize(iconSize_);
    setIconColor(iconColor_);
    setIconSeparatorWidth(iconSeparatorWidth_);
    setCloseButtonIcon(closeButtonIcon_);
    setCloseButtonIconSize(closeButtonIconSize_);
    setCloseButtonSize(closeButtonSize_);
    setCloseButtonAlignment(closeButtonAlignment_);
    setStayOnTop(stayOnTop_);
    setBackgroundColor(backgroundColor_);
    setTitleColor(titleColor_);
    setTextColor(textColor_);
    setBorderRadius(borderRadius_);
    setIconSeparatorColor(iconSeparatorColor_);
    setCloseButtonIconColor(closeButtonIconColor_);
    setDurationBarColor(durationBarColor_);
    setTitleFont(titleFont_);
    setTextFont(textFont_);

    setStyleSheet(TOAST_STYLE);
}

Toast::Toast(const ToastConfig& config, QWidget* parent)
    : Toast(parent)
{
    setDuration(config.duration)
        .setShowDurationBar(config.showDurationBar)
        .setTitle(config.title)
        .setText(config.text);

    if (!config.icon.isNull()) {
        setIcon(config.icon);
    }

    setShowIcon(config.showIcon)
        .setIconSize(config.iconSize)
        .setShowIconSeparator(config.showIconSeparator)
        .setIconSeparatorWidth(config.iconSeparatorWidth);

    if (!config.closeButtonIcon.isNull()) {
        setCloseButtonIcon(config.closeButtonIcon);
    }

    setShowCloseButton(config.showCloseButton)
        .setCloseButtonIconSize(config.closeButtonIconSize)
        .setCloseButtonSize(config.closeButtonSize)
        .setCloseButtonAlignment(config.closeButtonAlignment)
        .setFadeInDuration(config.fadeInDuration)
        .setFadeOutDuration(config.fadeOutDuration)
        .setResetDurationOnHover(config.resetDurationOnHover)
        .setStayOnTop(config.stayOnTop)
        .setBorderRadius(config.borderRadius);

    if (config.backgroundColor.isValid()) {
        setBackgroundColor(config.backgroundColor);
    }

    if (config.titleColor.isValid()) {
        setTitleColor(config.titleColor);
    }

    if (config.textColor.isValid()) {
        setTextColor(config.textColor);
    }

    if (config.iconColor.isValid()) {
        setIconColor(config.iconColor);
    }

    if (config.iconSeparatorColor.isValid()) {
        setIconSeparatorColor(config.iconSeparatorColor);
    }

    if (config.closeButtonIconColor.isValid()) {
        setCloseButtonIconColor(config.closeButtonIconColor);
    }

    if (config.durationBarColor.isValid()) {
        setDurationBarColor(config.durationBarColor);
    }

    setTitleFont(config.titleFont)
        .setTextFont(config.textFont)
        .setMargins(config.margins)
        .setIconMargins(config.iconMargins)
        .setIconSectionMargins(config.iconSectionMargins)
        .setTextSectionMargins(config.textSectionMargins)
        .setCloseButtonMargins(config.closeButtonMargins)
        .setTextSectionSpacing(config.textSectionSpacing);
}

Toast::~Toast()
{
    std::lock_guard<std::mutex> lock(ToastManager::mutex_);
    auto it = std::find(ToastManager::currentlyShown_.begin(),
        ToastManager::currentlyShown_.end(), this);
    if (it != ToastManager::currentlyShown_.end()) {
        ToastManager::currentlyShown_.erase(it);
    }

    auto queueIt = std::find(
        ToastManager::queue_.begin(), ToastManager::queue_.end(), this);
    if (queueIt != ToastManager::queue_.end()) {
        ToastManager::queue_.erase(queueIt);
    }
}

int Toast::getDuration() const
{
    return duration_;
}

bool Toast::isShowDurationBar() const
{
    return showDurationBar_;
}

QString Toast::getTitle() const
{
    return title_;
}

QString Toast::getText() const
{
    return text_;
}

QPixmap Toast::getIcon() const
{
    return icon_;
}

bool Toast::isShowIcon() const
{
    return showIcon_;
}

QSize Toast::getIconSize() const
{
    return iconSize_;
}

bool Toast::isShowIconSeparator() const
{
    return showIconSeparator_;
}

int Toast::getIconSeparatorWidth() const
{
    return iconSeparatorWidth_;
}

QPixmap Toast::getCloseButtonIcon() const
{
    return closeButtonIcon_;
}

bool Toast::isShowCloseButton() const
{
    return showCloseButton_;
}

QSize Toast::getCloseButtonIconSize() const
{
    return closeButtonIconSize_;
}

QSize Toast::getCloseButtonSize() const
{
    return closeButtonSize_;
}

ToastButtonAlignment Toast::getCloseButtonAlignment() const
{
    return closeButtonAlignment_;
}

int Toast::getFadeInDuration() const
{
    return fadeInDuration_;
}

int Toast::getFadeOutDuration() const
{
    return fadeOutDuration_;
}

bool Toast::isResetDurationOnHover() const
{
    return resetDurationOnHover_;
}

bool Toast::isStayOnTop() const
{
    return stayOnTop_;
}

int Toast::getBorderRadius() const
{
    return borderRadius_;
}

QColor Toast::getBackgroundColor() const
{
    return backgroundColor_;
}

QColor Toast::getTitleColor() const
{
    return titleColor_;
}

QColor Toast::getTextColor() const
{
    return textColor_;
}

QColor Toast::getIconColor() const
{
    return iconColor_;
}

QColor Toast::getIconSeparatorColor() const
{
    return iconSeparatorColor_;
}

QColor Toast::getCloseButtonIconColor() const
{
    return closeButtonIconColor_;
}

QColor Toast::getDurationBarColor() const
{
    return durationBarColor_;
}

QFont Toast::getTitleFont() const
{
    return titleFont_;
}

QFont Toast::getTextFont() const
{
    return textFont_;
}

QMargins Toast::getMargins() const
{
    return margins_;
}

QMargins Toast::getIconMargins() const
{
    return iconMargins_;
}

QMargins Toast::getIconSectionMargins() const
{
    return iconSectionMargins_;
}

QMargins Toast::getTextSectionMargins() const
{
    return textSectionMargins_;
}

QMargins Toast::getCloseButtonMargins() const
{
    return closeButtonMargins_;
}

int Toast::getTextSectionSpacing() const
{
    return textSectionSpacing_;
}

Toast& Toast::setDuration(int duration)
{
    if (!used_)
        duration_ = duration;
    return *this;
}

Toast& Toast::setShowDurationBar(bool enabled)
{
    if (!used_)
        showDurationBar_ = enabled;
    return *this;
}

Toast& Toast::setTitle(QString title)
{
    if (!used_) {
        title_ = title;
        titleLabel_->setText(title);
    }
    return *this;
}

Toast& Toast::setText(QString text)
{
    if (!used_) {
        text_ = text;
        textLabel_->setText(text);
    }
    return *this;
}

Toast& Toast::setIcon(QPixmap icon)
{
    if (!used_) {
        icon_ = icon;
        iconWidget_->setIcon(QIcon(icon));
        setIconColor(iconColor_);
    }
    return *this;
}

Toast& Toast::setIcon(ToastIcon icon)
{
    if (!used_) {
        icon_ = getIconFromEnum(icon);
        iconWidget_->setIcon(QIcon(icon_));
        setIconColor(iconColor_);
    }
    return *this;
}

Toast& Toast::setShowIcon(bool enabled)
{
    if (!used_)
        showIcon_ = enabled;
    return *this;
}

Toast& Toast::setIconSize(QSize size)
{
    if (!used_) {
        iconSize_ = size;
        iconWidget_->setFixedSize(size);
        iconWidget_->setIconSize(size);
        setIcon(icon_);
    }
    return *this;
}

Toast& Toast::setShowIconSeparator(bool enabled)
{
    if (!used_) {
        showIconSeparator_ = enabled;
        if (enabled) {
            iconSeparator_->setFixedWidth(iconSeparatorWidth_);
        } else {
            iconSeparator_->setFixedWidth(0);
        }
    }
    return *this;
}

Toast& Toast::setIconSeparatorWidth(int width)
{
    if (!used_) {
        iconSeparatorWidth_ = width;
        if (showIconSeparator_) {
            iconSeparator_->setFixedWidth(width);
        }
    }
    return *this;
}

Toast& Toast::setCloseButtonIcon(QPixmap icon)
{
    if (!used_) {
        closeButtonIcon_ = icon;
        closeButton_->setIcon(QIcon(icon));
        setCloseButtonIconColor(closeButtonIconColor_);
    }
    return *this;
}

Toast& Toast::setCloseButtonIcon(ToastIcon icon)
{
    if (!used_) {
        closeButtonIcon_ = getIconFromEnum(icon);
        closeButton_->setIcon(QIcon(closeButtonIcon_));
        setCloseButtonIconColor(closeButtonIconColor_);
    }
    return *this;
}

Toast& Toast::setShowCloseButton(bool enabled)
{
    if (!used_)
        showCloseButton_ = enabled;
    return *this;
}

Toast& Toast::setCloseButtonIconSize(QSize size)
{
    if (!used_) {
        closeButtonIconSize_ = size;
        closeButton_->setIconSize(size);
        setCloseButtonIcon(closeButtonIcon_);
    }
    return *this;
}

Toast& Toast::setCloseButtonSize(QSize size)
{
    if (!used_) {
        closeButtonSize_ = size;
        closeButton_->setFixedSize(size);
    }
    return *this;
}

Toast& Toast::setCloseButtonWidth(int width)
{
    if (!used_) {
        closeButtonSize_.setWidth(width);
        closeButton_->setFixedSize(closeButtonSize_);
    }
    return *this;
}

Toast& Toast::setCloseButtonHeight(int height)
{
    if (!used_) {
        closeButtonSize_.setHeight(height);
        closeButton_->setFixedSize(closeButtonSize_);
    }
    return *this;
}

Toast& Toast::setCloseButtonAlignment(ToastButtonAlignment alignment)
{
    if (!used_)
        closeButtonAlignment_ = alignment;
    return *this;
}

Toast& Toast::setFadeInDuration(int duration)
{
    if (!used_)
        fadeInDuration_ = duration;
    return *this;
}

Toast& Toast::setFadeOutDuration(int duration)
{
    if (!used_)
        fadeOutDuration_ = duration;
    return *this;
}

Toast& Toast::setResetDurationOnHover(bool enabled)
{
    if (!used_)
        resetDurationOnHover_ = enabled;
    return *this;
}

Toast& Toast::setStayOnTop(bool enabled)
{
    if (!used_) {
        stayOnTop_ = enabled;

        Qt::WindowFlags flags = Qt::WindowType::CustomizeWindowHint
            | Qt::WindowType::FramelessWindowHint;

        if (enabled) {
            flags |= Qt::WindowType::WindowStaysOnTopHint;
        }

        flags |= parent_ ? Qt::WindowType::Window : Qt::WindowType::Tool;

        setWindowFlags(flags);
    }
    return *this;
}

Toast& Toast::setBorderRadius(int borderRadius)
{
    if (!used_)
        borderRadius_ = borderRadius;
    return *this;
}

Toast& Toast::setBackgroundColor(QColor color)
{
    if (!used_)
        backgroundColor_ = color;
    return *this;
}

Toast& Toast::setTitleColor(QColor color)
{
    if (!used_)
        titleColor_ = color;
    return *this;
}

Toast& Toast::setTextColor(QColor color)
{
    if (!used_)
        textColor_ = color;
    return *this;
}

Toast& Toast::setIconColor(QColor color)
{
    if (!used_) {
        iconColor_ = color;
        QImage recoloredImage = recolorImage(
            iconWidget_->icon().pixmap(iconWidget_->iconSize()).toImage(),
            color);
        iconWidget_->setIcon(QIcon(QPixmap::fromImage(recoloredImage)));
    }
    return *this;
}

Toast& Toast::setIconSeparatorColor(QColor color)
{
    if (!used_)
        iconSeparatorColor_ = color;
    return *this;
}

Toast& Toast::setCloseButtonIconColor(QColor color)
{
    if (!used_) {
        closeButtonIconColor_ = color;
        QImage recoloredImage = recolorImage(
            closeButton_->icon().pixmap(closeButton_->iconSize()).toImage(),
            color);
        closeButton_->setIcon(QIcon(QPixmap::fromImage(recoloredImage)));
    }
    return *this;
}

Toast& Toast::setDurationBarColor(QColor color)
{
    if (!used_)
        durationBarColor_ = color;
    return *this;
}

Toast& Toast::setTitleFont(QFont font)
{
    if (!used_) {
        titleFont_ = font;
        titleLabel_->setFont(font);
    }
    return *this;
}

Toast& Toast::setTextFont(QFont font)
{
    if (!used_) {
        textFont_ = font;
        textLabel_->setFont(font);
    }
    return *this;
}

Toast& Toast::setMargins(QMargins margins)
{
    if (!used_)
        margins_ = margins;
    return *this;
}

Toast& Toast::setMarginLeft(int margin)
{
    if (!used_)
        margins_.setLeft(margin);
    return *this;
}

Toast& Toast::setMarginTop(int margin)
{
    if (!used_)
        margins_.setTop(margin);
    return *this;
}

Toast& Toast::setMarginRight(int margin)
{
    if (!used_)
        margins_.setRight(margin);
    return *this;
}

Toast& Toast::setMarginBottom(int margin)
{
    if (!used_)
        margins_.setBottom(margin);
    return *this;
}

Toast& Toast::setIconMargins(QMargins margins)
{
    if (!used_)
        iconMargins_ = margins;
    return *this;
}

Toast& Toast::setIconMarginLeft(int margin)
{
    if (!used_)
        iconMargins_.setLeft(margin);
    return *this;
}

Toast& Toast::setIconMarginTop(int margin)
{
    if (!used_)
        iconMargins_.setTop(margin);
    return *this;
}

Toast& Toast::setIconMarginRight(int margin)
{
    if (!used_)
        iconMargins_.setRight(margin);
    return *this;
}

Toast& Toast::setIconMarginBottom(int margin)
{
    if (!used_)
        iconMargins_.setBottom(margin);
    return *this;
}

Toast& Toast::setIconSectionMargins(QMargins margins)
{
    if (!used_)
        iconSectionMargins_ = margins;
    return *this;
}

Toast& Toast::setIconSectionMarginLeft(int margin)
{
    if (!used_)
        iconSectionMargins_.setLeft(margin);
    return *this;
}

Toast& Toast::setIconSectionMarginTop(int margin)
{
    if (!used_)
        iconSectionMargins_.setTop(margin);
    return *this;
}

Toast& Toast::setIconSectionMarginRight(int margin)
{
    if (!used_)
        iconSectionMargins_.setRight(margin);
    return *this;
}

Toast& Toast::setIconSectionMarginBottom(int margin)
{
    if (!used_)
        iconSectionMargins_.setBottom(margin);
    return *this;
}

Toast& Toast::setTextSectionMargins(QMargins margins)
{
    if (!used_)
        textSectionMargins_ = margins;
    return *this;
}

Toast& Toast::setTextSectionMarginLeft(int margin)
{
    if (!used_)
        textSectionMargins_.setLeft(margin);
    return *this;
}

Toast& Toast::setTextSectionMarginTop(int margin)
{
    if (!used_)
        textSectionMargins_.setTop(margin);
    return *this;
}

Toast& Toast::setTextSectionMarginRight(int margin)
{
    if (!used_)
        textSectionMargins_.setRight(margin);
    return *this;
}

Toast& Toast::setTextSectionMarginBottom(int margin)
{
    if (!used_)
        textSectionMargins_.setBottom(margin);
    return *this;
}

Toast& Toast::setCloseButtonMargins(QMargins margins)
{
    if (!used_)
        closeButtonMargins_ = margins;
    return *this;
}

Toast& Toast::setCloseButtonMarginLeft(int margin)
{
    if (!used_)
        closeButtonMargins_.setLeft(margin);
    return *this;
}

Toast& Toast::setCloseButtonMarginTop(int margin)
{
    if (!used_)
        closeButtonMargins_.setTop(margin);
    return *this;
}

Toast& Toast::setCloseButtonMarginRight(int margin)
{
    if (!used_)
        closeButtonMargins_.setRight(margin);
    return *this;
}

Toast& Toast::setCloseButtonMarginBottom(int margin)
{
    if (!used_)
        closeButtonMargins_.setBottom(margin);
    return *this;
}

Toast& Toast::setTextSectionSpacing(int spacing)
{
    if (!used_)
        textSectionSpacing_ = spacing;
    return *this;
}

Toast& Toast::setFixedSize(QSize size)
{
    if (!used_) {
        QDialog::setMinimumSize(size);
        QDialog::setMaximumSize(size);
    }
    return *this;
}

Toast& Toast::setFixedSize(int width, int height)
{
    return setFixedSize(QSize(width, height));
}

Toast& Toast::setFixedWidth(int width)
{
    if (!used_) {
        QDialog::setMinimumWidth(width);
        QDialog::setMaximumWidth(width);
    }
    return *this;
}

Toast& Toast::setFixedHeight(int height)
{
    if (!used_) {
        QDialog::setMinimumHeight(height);
        QDialog::setMaximumHeight(height);
    }
    return *this;
}

Toast& Toast::applyPreset(ToastPreset preset)
{
    if (used_)
        return *this;

    switch (preset) {
    case ToastPreset::SUCCESS:
    case ToastPreset::SUCCESS_DARK:
        setIcon(ToastIcon::SUCCESS);
        setIconColor(successAccentColor_);
        setDurationBarColor(successAccentColor_);
        break;

    case ToastPreset::WARNING:
    case ToastPreset::WARNING_DARK:
        setIcon(ToastIcon::WARNING);
        setIconColor(warningAccentColor_);
        setDurationBarColor(warningAccentColor_);
        break;

    case ToastPreset::TOAST_ERROR:
    case ToastPreset::ERROR_DARK:
        setIcon(ToastIcon::TOAST_ERROR);
        setIconColor(errorAccentColor_);
        setDurationBarColor(errorAccentColor_);
        break;

    case ToastPreset::INFORMATION:
    case ToastPreset::INFORMATION_DARK:
        setIcon(ToastIcon::INFORMATION);
        setIconColor(informationAccentColor_);
        setDurationBarColor(informationAccentColor_);
        break;
    }

    switch (preset) {
    case ToastPreset::SUCCESS:
    case ToastPreset::WARNING:
    case ToastPreset::TOAST_ERROR:
    case ToastPreset::INFORMATION:
        setBackgroundColor(defaultBackgroundColor_);
        setCloseButtonIconColor(defaultCloseButtonIconColor_);
        setIconSeparatorColor(defaultIconSeparatorColor_);
        setTitleColor(defaultTitleColor_);
        setTextColor(defaultTextColor_);
        break;

    case ToastPreset::SUCCESS_DARK:
    case ToastPreset::WARNING_DARK:
    case ToastPreset::ERROR_DARK:
    case ToastPreset::INFORMATION_DARK:
        setBackgroundColor(defaultBackgroundColorDark_);
        setCloseButtonIconColor(defaultCloseButtonIconColorDark_);
        setIconSeparatorColor(defaultIconSeparatorColorDark_);
        setTitleColor(defaultTitleColorDark_);
        setTextColor(defaultTextColorDark_);
        break;
    }

    return setShowDurationBar(true)
        .setShowIcon(true)
        .setShowIconSeparator(true)
        .setIconSeparatorWidth(2);
}

void Toast::show()
{
    if (used_)
        return;

    std::lock_guard<std::mutex> lock(ToastManager::mutex_);

    if (ToastManager::maximumOnScreen_ > ToastManager::currentlyShown_.size()) {
        used_ = true;
        ToastManager::currentlyShown_.push_back(this);

        setupUI();

        if (duration_ != 0) {
            durationTimer_->start(duration_);

            if (showDurationBar_) {
                durationBarTimer_->start(durationBarUpdateInterval_);
            }
        }

        QPoint position = calculatePosition();

        if (ToastManager::currentlyShown_.size() > 1) {
            Toast* predecessorToast = getPredecessorToast();
            QPoint predecessorTarget = predecessorToast->calculatePosition();
            int predecessorTargetDifferenceY
                = abs(predecessorToast->y() - predecessorTarget.y());

            if (ToastManager::position_ == ToastPosition::BOTTOM_RIGHT
                || ToastManager::position_ == ToastPosition::BOTTOM_LEFT
                || ToastManager::position_ == ToastPosition::BOTTOM_MIDDLE) {
                move(position.x(),
                    position.y() - (int)(height() / 1.5)
                        - predecessorTargetDifferenceY);
            } else {
                move(position.x(),
                    position.y() + (int)(height() / 1.5)
                        + predecessorTargetDifferenceY);
            }

            auto posAnimation
                = std::make_unique<QPropertyAnimation>(this, "pos");
            posAnimation->setEndValue(QPoint(position.x(), position.y()));
            posAnimation->setDuration(fadeInDuration_);
            posAnimation->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            move(position);
        }

        QDialog::show();
        auto fadeInAnimation = std::make_unique<QPropertyAnimation>(
            opacityEffect_.get(), "opacity");
        fadeInAnimation->setDuration(fadeInDuration_);
        fadeInAnimation->setStartValue(0);
        fadeInAnimation->setEndValue(1);
        fadeInAnimation->start(QAbstractAnimation::DeleteWhenStopped);

        if (parent_) {
            parent_->activateWindow();
        }

        for (auto toast : ToastManager::currentlyShown_) {
            if (toast != this) {
                toast->updatePositionXY();
            }
        }
    } else {
        ToastManager::queue_.push_back(this);
    }
}

void Toast::hide()
{
    if (!fadingOut_) {
        if (duration_ != 0) {
            durationTimer_->stop();
        }
        fadeOut();
    }
}

void Toast::hide_()
{
    close();

    {
        std::lock_guard<std::mutex> lock(ToastManager::mutex_);
        auto it = std::find(ToastManager::currentlyShown_.begin(),
            ToastManager::currentlyShown_.end(), this);
        if (it != ToastManager::currentlyShown_.end()) {
            ToastManager::currentlyShown_.erase(it);
            elapsedTime_ = 0;
            fadingOut_ = false;

            Q_EMIT closed();

            for (auto toast : ToastManager::currentlyShown_) {
                toast->updatePositionY();
            }

            QTimer::singleShot(
                fadeInDuration_, this, &Toast::deleteAndShowNextInQueue);
        }
    }
}

void Toast::fadeOut()
{
    fadingOut_ = true;
    auto fadeOutAnimation
        = std::make_unique<QPropertyAnimation>(opacityEffect_.get(), "opacity");
    fadeOutAnimation->setDuration(fadeOutDuration_);
    fadeOutAnimation->setStartValue(1);
    fadeOutAnimation->setEndValue(0);

    connect(fadeOutAnimation.get(), &QPropertyAnimation::finished, this,
        &Toast::hide_);

    fadeOutAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Toast::updateDurationBar()
{
    elapsedTime_ += durationBarUpdateInterval_;

    if (elapsedTime_ >= duration_) {
        durationBarTimer_->stop();
        return;
    }

    double newChunkWidth = floor(durationBarContainer_->width()
        - (double)elapsedTime_ / (double)duration_
            * durationBarContainer_->width());
    durationBarChunk_->setFixedWidth(newChunkWidth);
}

void Toast::deleteAndShowNextInQueue()
{
    ToastManager::showNextInQueue();
    deleteLater();
}

void Toast::enterEvent(QEvent* event)
{
    if (duration_ != 0 && durationTimer_->isActive() && resetDurationOnHover_) {
        durationTimer_->stop();

        if (showDurationBar_) {
            durationBarTimer_->stop();
            durationBarChunk_->setFixedWidth(notification_->width());
            elapsedTime_ = 0;
        }
    }
}

void Toast::leaveEvent(QEvent* event)
{
    if (duration_ != 0 && !durationTimer_->isActive()
        && resetDurationOnHover_) {
        durationTimer_->start(duration_);

        if (showDurationBar_) {
            durationBarTimer_->start(durationBarUpdateInterval_);
        }
    }
}

void Toast::setupUI()
{
    updateStylesheet();

    QFontMetrics titleFontMetrics(titleFont_);
    int titleWidth = titleFontMetrics.boundingRect(title_).width() + 1;
    int titleHeight = titleFontMetrics.boundingRect(title_).height();

    QFontMetrics textFontMetrics(textFont_);
    int textWidth = textFontMetrics.boundingRect(text_).width() + 1;
    int textHeight = textFontMetrics.boundingRect(text_).height();

    int textSectionSpacing
        = (title_.isEmpty() || text_.isEmpty()) ? 0 : textSectionSpacing_;

    int textSectionHeight = textSectionMargins_.top() + titleHeight
        + textSectionSpacing + textHeight + textSectionMargins_.bottom();

    int durationBarHeight
        = showDurationBar_ ? durationBarContainer_->height() : 0;

    int iconSectionWidth = 0;
    int iconSectionHeight = 0;

    if (showIcon_) {
        iconSectionWidth = iconSectionMargins_.left() + iconMargins_.left()
            + iconWidget_->width() + iconMargins_.right()
            + iconSeparator_->width() + iconSectionMargins_.right();

        iconSectionHeight = iconSectionMargins_.top() + iconMargins_.top()
            + iconWidget_->height() + iconMargins_.bottom()
            + iconSectionMargins_.bottom();
    }

    int closeButtonWidth = showCloseButton_ ? closeButton_->width() : 0;
    int closeButtonHeight = showCloseButton_ ? closeButton_->height() : 0;
    QMargins closeButtonMargins
        = showCloseButton_ ? closeButtonMargins_ : QMargins(0, 0, 0, 0);

    int closeButtonSectionHeight = closeButtonMargins.top() + closeButtonHeight
        + closeButtonMargins.bottom();

    int width = margins_.left() + iconSectionWidth + textSectionMargins_.left()
        + std::max(titleWidth, textWidth) + textSectionMargins_.right()
        + closeButtonMargins.left() + closeButtonWidth
        + closeButtonMargins.right() + margins_.right();

    int height = margins_.top()
        + std::max(
            { iconSectionHeight, textSectionHeight, closeButtonSectionHeight })
        + margins_.bottom() + durationBarHeight;

    int forcedAdditionalHeight = 0;
    int forcedReducedHeight = 0;

    if (width > maximumWidth()) {
        int newTitleWidth
            = std::max(titleWidth, textWidth) - (width - maximumWidth());
        if (newTitleWidth > 0) {
            titleWidth = newTitleWidth;
        }

        int newTextWidth
            = std::max(titleWidth, textWidth) - (width - maximumWidth());
        if (newTextWidth > 0) {
            textWidth = newTextWidth;
        }

        titleLabel_->setMinimumWidth(titleWidth);
        titleLabel_->setWordWrap(true);
        if (!title_.isEmpty()) {
            titleHeight = titleLabel_->sizeHint().height();
        }
        titleLabel_->setFixedSize(titleWidth, titleHeight);

        textLabel_->setMinimumWidth(textWidth);
        textLabel_->setWordWrap(true);
        if (!text_.isEmpty()) {
            textHeight = textLabel_->sizeHint().height();
        }
        textLabel_->setFixedSize(textWidth, textHeight);

        width = maximumWidth();

        textSectionHeight = textSectionMargins_.top() + titleHeight
            + textSectionSpacing + textHeight + textSectionMargins_.bottom();

        height = margins_.top()
            + std::max({ iconSectionHeight, textSectionHeight,
                closeButtonSectionHeight })
            + margins_.bottom() + durationBarHeight;
    }

    if (height < minimumHeight()) {
        titleLabel_->setWordWrap(true);
        textLabel_->setWordWrap(true);

        titleWidth = titleLabel_->fontMetrics()
                         .boundingRect(QRect(0, 0, 0, 0),
                             Qt::TextFlag::TextWordWrap, title_)
                         .width();

        textWidth = textLabel_->fontMetrics()
                        .boundingRect(QRect(0, 0, 0, 0),
                            Qt::TextFlag::TextWordWrap, text_)
                        .width();

        int tempWidth = std::max(titleWidth, textWidth);

        titleWidth = titleLabel_->fontMetrics()
                         .boundingRect(QRect(0, 0, tempWidth, 0),
                             Qt::TextFlag::TextWordWrap, title_)
                         .width();
        if (!title_.isEmpty()) {
            titleHeight = titleLabel_->fontMetrics()
                              .boundingRect(QRect(0, 0, tempWidth, 0),
                                  Qt::TextFlag::TextWordWrap, title_)
                              .height();
        }

        textWidth = textLabel_->fontMetrics()
                        .boundingRect(QRect(0, 0, tempWidth, 0),
                            Qt::TextFlag::TextWordWrap, text_)
                        .width();
        if (!text_.isEmpty()) {
            textHeight = textLabel_->fontMetrics()
                             .boundingRect(QRect(0, 0, tempWidth, 0),
                                 Qt::TextFlag::TextWordWrap, text_)
                             .height();
        }

        textSectionHeight = textSectionMargins_.top() + titleHeight
            + textSectionSpacing + textHeight + textSectionMargins_.bottom();

        height = margins_.top()
            + std::max({ iconSectionHeight, textSectionHeight,
                closeButtonSectionHeight })
            + margins_.bottom() + durationBarHeight;

        while (tempWidth <= width) {
            int tempTitleWidth = titleLabel_->fontMetrics()
                                     .boundingRect(QRect(0, 0, tempWidth, 0),
                                         Qt::TextFlag::TextWordWrap, title_)
                                     .width();

            int tempTitleHeight = titleLabel_->fontMetrics()
                                      .boundingRect(QRect(0, 0, tempWidth, 0),
                                          Qt::TextFlag::TextWordWrap, title_)
                                      .height();

            int tempTextWidth = textLabel_->fontMetrics()
                                    .boundingRect(QRect(0, 0, tempWidth, 0),
                                        Qt::TextFlag::TextWordWrap, text_)
                                    .width();

            int tempTextHeight = textLabel_->fontMetrics()
                                     .boundingRect(QRect(0, 0, tempWidth, 0),
                                         Qt::TextFlag::TextWordWrap, text_)
                                     .height();

            if (title_.isEmpty()) {
                tempTitleHeight = 0;
            }

            if (text_.isEmpty()) {
                tempTextHeight = 0;
            }

            int tempTextSectionHeight = textSectionMargins_.top()
                + tempTitleHeight + textSectionSpacing + tempTextHeight
                + textSectionMargins_.bottom();

            int tempHeight = margins_.top()
                + std::max({ iconSectionHeight, tempTextSectionHeight,
                    closeButtonSectionHeight })
                + margins_.bottom() + durationBarHeight;

            if (tempHeight >= minimumHeight()) {
                titleWidth = tempTitleWidth;
                titleHeight = tempTitleHeight;
                textWidth = tempTextWidth;
                textHeight = tempTextHeight;
                textSectionHeight = tempTextSectionHeight;
                height = tempHeight;
                tempWidth += 1;
            } else {
                break;
            }
        }

        width = margins_.left() + iconSectionWidth + textSectionMargins_.left()
            + std::max(titleWidth, textWidth) + textSectionMargins_.right()
            + closeButtonMargins.left() + closeButtonWidth
            + closeButtonMargins.right() + margins_.right();

        if (height < minimumHeight()) {
            forcedAdditionalHeight = minimumHeight() - height;
            height = minimumHeight();
        }
    }

    if (width < minimumWidth()) {
        width = minimumWidth();
    }

    if (height > maximumHeight()) {
        forcedReducedHeight = height - maximumHeight();
        height = maximumHeight();
    }

    int totalWidth = width + dropShadowSize_ * 2;
    int totalHeight = height + dropShadowSize_ * 2;

    dropShadowLayer1_->resize(totalWidth, totalHeight);
    dropShadowLayer1_->move(0, 0);
    dropShadowLayer2_->resize(totalWidth - 2, totalHeight - 2);
    dropShadowLayer2_->move(1, 1);
    dropShadowLayer3_->resize(totalWidth - 4, totalHeight - 4);
    dropShadowLayer3_->move(2, 2);
    dropShadowLayer4_->resize(totalWidth - 6, totalHeight - 6);
    dropShadowLayer4_->move(3, 3);
    dropShadowLayer5_->resize(totalWidth - 8, totalHeight - 8);
    dropShadowLayer5_->move(4, 4);

    QDialog::setFixedSize(totalWidth, totalHeight);
    notification_->setFixedSize(width, height);
    notification_->move(dropShadowSize_, dropShadowSize_);
    notification_->raise();

    int heightIconSectionHeightDifference
        = std::max({ iconSectionHeight, textSectionHeight,
              closeButtonSectionHeight })
        - iconSectionHeight;

    if (showIcon_) {
        iconWidget_->move(
            margins_.left() + iconSectionMargins_.left() + iconMargins_.left(),
            margins_.top() + iconSectionMargins_.top() + iconMargins_.top()
                + ceil(heightIconSectionHeightDifference / 2)
                - floor(forcedReducedHeight / 2));

        iconSeparator_->setFixedHeight(textSectionHeight);
        iconSeparator_->move(margins_.left() + iconSectionMargins_.left()
                + iconMargins_.left() + iconWidget_->width()
                + iconMargins_.right(),
            margins_.top() + iconSectionMargins_.top()
                + ceil(forcedAdditionalHeight / 2)
                - floor(forcedReducedHeight / 2));
    } else {
        iconWidget_->setVisible(false);
        iconSeparator_->setVisible(false);
    }

    int heightTextSectionHeightDifference
        = std::max({ iconSectionHeight, textSectionHeight,
              closeButtonSectionHeight })
        - textSectionHeight;

    titleLabel_->setFixedSize(std::max(titleWidth, textWidth), titleHeight);
    textLabel_->setFixedSize(std::max(titleWidth, textWidth), textHeight);

    if (showIcon_) {
        titleLabel_->move(margins_.left() + iconSectionMargins_.left()
                + iconMargins_.left() + iconWidget_->width()
                + iconMargins_.right() + iconSeparator_->width()
                + iconSectionMargins_.right() + textSectionMargins_.left(),
            margins_.top() + textSectionMargins_.top()
                + ceil(heightTextSectionHeightDifference / 2)
                + ceil(forcedAdditionalHeight / 2)
                - floor(forcedReducedHeight / 2));

        textLabel_->move(margins_.left() + iconSectionMargins_.left()
                + iconMargins_.left() + iconWidget_->width()
                + iconMargins_.right() + iconSeparator_->width()
                + iconSectionMargins_.right() + textSectionMargins_.left(),
            margins_.top() + textSectionMargins_.top() + titleHeight
                + textSectionSpacing
                + ceil(heightTextSectionHeightDifference / 2)
                + ceil(forcedAdditionalHeight / 2)
                - floor(forcedReducedHeight / 2));
    } else {
        titleLabel_->move(margins_.left() + textSectionMargins_.left(),
            margins_.top() + textSectionMargins_.top()
                + ceil(heightTextSectionHeightDifference / 2)
                + ceil(forcedAdditionalHeight / 2)
                - floor(forcedReducedHeight / 2));

        textLabel_->move(margins_.left() + textSectionMargins_.left(),
            margins_.top() + textSectionMargins_.top() + titleHeight
                + textSectionSpacing
                + ceil(heightTextSectionHeightDifference / 2)
                + ceil(forcedAdditionalHeight / 2)
                - floor(forcedReducedHeight / 2));
    }

    if (title_.isEmpty() && !text_.isEmpty()) {
        textLabel_->move(textLabel_->x(),
            (int)((height - textHeight - durationBarHeight) / 2));
    } else if (!title_.isEmpty() && text_.isEmpty()) {
        titleLabel_->move(titleLabel_->x(),
            (int)((height - titleHeight - durationBarHeight) / 2));
    }

    if (closeButtonAlignment_ == ToastButtonAlignment::TOP) {
        closeButton_->move(width - closeButtonWidth - closeButtonMargins.right()
                - margins_.right(),
            margins_.top() + closeButtonMargins.top());
    } else if (closeButtonAlignment_ == ToastButtonAlignment::MIDDLE) {
        closeButton_->move(width - closeButtonWidth - closeButtonMargins.right()
                - margins_.right(),
            ceil((height - closeButtonHeight - durationBarHeight) / 2));
    } else if (closeButtonAlignment_ == ToastButtonAlignment::BOTTOM) {
        closeButton_->move(width - closeButtonWidth - closeButtonMargins.right()
                - margins_.right(),
            height - closeButtonHeight - margins_.bottom()
                - closeButtonMargins.bottom() - durationBarHeight);
    }

    if (!showCloseButton_) {
        closeButton_->setVisible(false);
    }

    if (showDurationBar_) {
        durationBarContainer_->setFixedWidth(width);
        durationBarContainer_->move(0, height - durationBarHeight);
        durationBar_->setFixedWidth(width);
        durationBarChunk_->setFixedWidth(width);
    } else {
        durationBarContainer_->setVisible(false);
    }
}

void Toast::updateStylesheet()
{
    notification_->setStyleSheet(QString("background: %1; border-radius: %2px;")
            .arg(backgroundColor_.name())
            .arg(borderRadius_));

    durationBar_->setStyleSheet(
        QString("background: rgba(%1, %2, %3, 100); border-radius: %4px;")
            .arg(durationBarColor_.red())
            .arg(durationBarColor_.green())
            .arg(durationBarColor_.blue())
            .arg(borderRadius_));

    durationBarChunk_->setStyleSheet(QString(
        "background: rgba(%1, %2, %3, 255); "
        "border-bottom-left-radius: %4px; border-bottom-right-radius: %5px;")
            .arg(durationBarColor_.red())
            .arg(durationBarColor_.green())
            .arg(durationBarColor_.blue())
            .arg(borderRadius_)
            .arg(duration_ == 0 ? borderRadius_ : 0));

    iconSeparator_->setStyleSheet(
        QString("background: %1;").arg(iconSeparatorColor_.name()));

    titleLabel_->setStyleSheet(QString("color: %1;").arg(titleColor_.name()));
    textLabel_->setStyleSheet(QString("color: %1;").arg(textColor_.name()));
}

void Toast::updatePositionXY()
{
    QPoint position = calculatePosition();

    auto posAnimation = std::make_unique<QPropertyAnimation>(this, "pos");
    posAnimation->setEndValue(position);
    posAnimation->setDuration(updatePositionDuration_);
    posAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Toast::updatePositionX()
{
    QPoint position = calculatePosition();

    auto posAnimation = std::make_unique<QPropertyAnimation>(this, "pos");
    posAnimation->setEndValue(QPoint(position.x(), y()));
    posAnimation->setDuration(updatePositionDuration_);
    posAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Toast::updatePositionY()
{
    QPoint position = calculatePosition();

    auto posAnimation = std::make_unique<QPropertyAnimation>(this, "pos");
    posAnimation->setEndValue(QPoint(x(), position.y()));
    posAnimation->setDuration(updatePositionDuration_);
    posAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

QPoint Toast::calculatePosition() const
{
    std::lock_guard<std::mutex> lock(ToastManager::mutex_);

    int offsetY = 0;

    for (Toast* toast : ToastManager::currentlyShown_) {
        if (toast == this) {
            break;
        }
        offsetY += toast->notification_->height() + ToastManager::spacing_;
    }

    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    QScreen* currentScreen = nullptr;

    if (ToastManager::fixedScreen_.has_value()) {
        currentScreen = ToastManager::fixedScreen_.value();
    } else if (ToastManager::alwaysOnMainScreen_ || !parent_) {
        currentScreen = primaryScreen;
    } else {
        QList<QScreen*> screens = QGuiApplication::screens();

        for (QScreen* screen : screens) {
            if (parent_->geometry().intersects(screen->geometry())) {
                if (!currentScreen) {
                    currentScreen = screen;
                } else {
                    currentScreen = primaryScreen;
                    break;
                }
            }
        }
    }

    int x = 0;
    int y = 0;

    switch (ToastManager::position_) {
    case ToastPosition::BOTTOM_RIGHT:
        x = currentScreen->geometry().width() - notification_->width()
            - ToastManager::offsetX_ + currentScreen->geometry().x();
        y = currentScreen->geometry().height() - notification_->height()
            - ToastManager::offsetY_ + currentScreen->geometry().y() - offsetY;
        break;

    case ToastPosition::BOTTOM_LEFT:
        x = currentScreen->geometry().x() + ToastManager::offsetX_;
        y = currentScreen->geometry().height() - notification_->height()
            - ToastManager::offsetY_ + currentScreen->geometry().y() - offsetY;
        break;

    case ToastPosition::BOTTOM_MIDDLE:
        x = (int)(currentScreen->geometry().x()
            + currentScreen->geometry().width() / 2
            - notification_->width() / 2);
        y = currentScreen->geometry().height() - notification_->height()
            - ToastManager::offsetY_ + currentScreen->geometry().y() - offsetY;
        break;

    case ToastPosition::TOP_RIGHT:
        x = currentScreen->geometry().width() - notification_->width()
            - ToastManager::offsetX_ + currentScreen->geometry().x();
        y = currentScreen->geometry().y() + ToastManager::offsetY_ + offsetY;
        break;

    case ToastPosition::TOP_LEFT:
        x = currentScreen->geometry().x() + ToastManager::offsetX_;
        y = currentScreen->geometry().y() + ToastManager::offsetY_ + offsetY;
        break;

    case ToastPosition::TOP_MIDDLE:
        x = (int)(currentScreen->geometry().x()
            + currentScreen->geometry().width() / 2
            - notification_->width() / 2);
        y = currentScreen->geometry().y() + ToastManager::offsetY_ + offsetY;
        break;

    case ToastPosition::CENTER:
        x = (int)(currentScreen->geometry().x()
            + currentScreen->geometry().width() / 2
            - notification_->width() / 2);
        y = (int)(currentScreen->geometry().y()
            + currentScreen->geometry().height() / 2
            - notification_->height() / 2 + offsetY);
        break;
    }

    x -= dropShadowSize_;
    y -= dropShadowSize_;

    return QPoint(x, y);
}

Toast* Toast::getPredecessorToast() const
{
    std::lock_guard<std::mutex> lock(ToastManager::mutex_);
    Toast* predecessorToast = nullptr;

    for (Toast* toast : ToastManager::currentlyShown_) {
        if (toast == this) {
            return predecessorToast;
        }
        predecessorToast = toast;
    }

    return predecessorToast;
}

QString Toast::getCurrentDirectory()
{
    return QFileInfo(__FILE__).absolutePath();
}

QImage Toast::recolorImage(QImage image, QColor color)
{
    for (int x = 0; x < image.width(); x++) {
        for (int y = 0; y < image.height(); y++) {
            QColor currentColor = image.pixelColor(x, y);

            QColor newColor = QColor::fromRgba(qRgba(color.red(), color.green(),
                color.blue(), currentColor.alpha()));
            image.setPixelColor(x, y, newColor);
        }
    }
    return image;
}

QPixmap Toast::getIconFromEnum(ToastIcon enumIcon)
{
    QString currentDirectory = getCurrentDirectory();

    switch (enumIcon) {
    case ToastIcon::SUCCESS:
        return QPixmap(currentDirectory + ":/Toast/success");
    case ToastIcon::WARNING:
        return QPixmap(currentDirectory + ":/Toast/warning");
    case ToastIcon::TOAST_ERROR:
        return QPixmap(currentDirectory + ":/Toast/error");
    case ToastIcon::INFORMATION:
        return QPixmap(currentDirectory + ":/Toast/information");
    case ToastIcon::CLOSE:
        return QPixmap(currentDirectory + ":/Toast/close");
    default:
        return QPixmap();
    }
}

}
