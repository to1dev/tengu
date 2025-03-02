// SPDX-License-Identifier: AGPL-3.0-or-later
/*
 * Copyright (C) 2024 Niklas Henning <https://github.com/niklashenning/qt-toast>
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

#pragma once

#include <algorithm>
#include <cmath>
#include <deque>
#include <execution>
#include <memory>
#include <mutex>
#include <optional>
#include <ranges>
#include <string_view>

#include <QDialog>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsOpacityEffect>
#include <QGuiApplication>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScreen>
#include <QStandardPaths>
#include <QTimer>

namespace Daitengu::Components {

enum class ToastPreset {
    SUCCESS,
    WARNING,
    TOAST_ERROR,
    INFORMATION,
    SUCCESS_DARK,
    WARNING_DARK,
    ERROR_DARK,
    INFORMATION_DARK,
};

enum class ToastIcon {
    SUCCESS,
    WARNING,
    TOAST_ERROR,
    INFORMATION,
    CLOSE,
};

enum class ToastPosition {
    BOTTOM_LEFT,
    BOTTOM_MIDDLE,
    BOTTOM_RIGHT,
    TOP_LEFT,
    TOP_MIDDLE,
    TOP_RIGHT,
    CENTER,
};

enum class ToastButtonAlignment {
    TOP,
    MIDDLE,
    BOTTOM,
};

struct ToastConfig {
    int duration { 5000 };
    bool showDurationBar { true };
    QString title;
    QString text;
    QPixmap icon;
    bool showIcon { false };
    QSize iconSize { 18, 18 };
    bool showIconSeparator { true };
    int iconSeparatorWidth { 2 };
    QPixmap closeButtonIcon;
    bool showCloseButton { true };
    QSize closeButtonIconSize { 10, 10 };
    QSize closeButtonSize { 24, 24 };
    ToastButtonAlignment closeButtonAlignment { ToastButtonAlignment::TOP };
    int fadeInDuration { 250 };
    int fadeOutDuration { 250 };
    bool resetDurationOnHover { true };
    bool stayOnTop { true };
    int borderRadius { 0 };
    QColor backgroundColor;
    QColor titleColor;
    QColor textColor;
    QColor iconColor;
    QColor iconSeparatorColor;
    QColor closeButtonIconColor;
    QColor durationBarColor;
    QFont titleFont { "Arial", 9, QFont::Weight::Bold };
    QFont textFont { "Arial", 9 };
    QMargins margins { 20, 18, 10, 18 };
    QMargins iconMargins { 0, 0, 15, 0 };
    QMargins iconSectionMargins { 0, 0, 15, 0 };
    QMargins textSectionMargins { 0, 0, 15, 0 };
    QMargins closeButtonMargins { 0, -8, 0, -8 };
    int textSectionSpacing { 8 };
};

class Toast;

class ToastManager {
public:
    [[nodiscard]] static int getMaximumOnScreen();
    [[nodiscard]] static int getSpacing();
    [[nodiscard]] static QPoint getOffset();
    [[nodiscard]] static int getOffsetX();
    [[nodiscard]] static int getOffsetY();
    [[nodiscard]] static bool isAlwaysOnMainScreen();
    [[nodiscard]] static std::optional<std::reference_wrapper<QScreen>>
    getFixedScreen();
    [[nodiscard]] static ToastPosition getPosition();
    [[nodiscard]] static int getCount();
    [[nodiscard]] static int getVisibleCount();
    [[nodiscard]] static int getQueuedCount();

    static void setMaximumOnScreen(int maximum);
    static void setSpacing(int spacing);
    static void setOffset(int x, int y);
    static void setOffsetX(int offsetX);
    static void setOffsetY(int offsetY);
    static void setAlwaysOnMainScreen(bool enabled);
    static void setFixedScreen(QScreen* screen);
    static void setPosition(const ToastPosition& position);
    static void reset();

    static void addToast(Toast* toast);
    static void removeToast(Toast* toast);
    static void showNextInQueue();

private:
    static constinit int maximumOnScreen_;
    static constinit int spacing_;
    static constinit int offsetX_;
    static constinit int offsetY_;
    static constinit bool alwaysOnMainScreen_;
    static std::optional<QScreen*> fixedScreen_;
    static constinit ToastPosition position_;
    static std::deque<Toast*> currentlyShown_;
    static std::deque<Toast*> queue_;
    static std::mutex mutex_;

    static void updateCurrentlyShowingPositionXY();
    static void updateCurrentlyShowingPositionX();
    static void updateCurrentlyShowingPositionY();

    friend class Toast;
};

class Toast : public QDialog {
    Q_OBJECT

public:
    explicit Toast(QWidget* parent = nullptr);
    explicit Toast(const ToastConfig& config, QWidget* parent = nullptr);
    ~Toast() override;

    Toast(const Toast&) = delete;
    Toast& operator=(const Toast&) = delete;

    Toast(Toast&&) noexcept = default;
    Toast& operator=(Toast&&) noexcept = default;

    [[nodiscard]] int getDuration() const;
    [[nodiscard]] bool isShowDurationBar() const;
    [[nodiscard]] QString getTitle() const;
    [[nodiscard]] QString getText() const;
    [[nodiscard]] QPixmap getIcon() const;
    [[nodiscard]] bool isShowIcon() const;
    [[nodiscard]] QSize getIconSize() const;
    [[nodiscard]] bool isShowIconSeparator() const;
    [[nodiscard]] int getIconSeparatorWidth() const;
    [[nodiscard]] QPixmap getCloseButtonIcon() const;
    [[nodiscard]] bool isShowCloseButton() const;
    [[nodiscard]] QSize getCloseButtonIconSize() const;
    [[nodiscard]] QSize getCloseButtonSize() const;
    [[nodiscard]] ToastButtonAlignment getCloseButtonAlignment() const;
    [[nodiscard]] int getFadeInDuration() const;
    [[nodiscard]] int getFadeOutDuration() const;
    [[nodiscard]] bool isResetDurationOnHover() const;
    [[nodiscard]] bool isStayOnTop() const;
    [[nodiscard]] int getBorderRadius() const;
    [[nodiscard]] QColor getBackgroundColor() const;
    [[nodiscard]] QColor getTitleColor() const;
    [[nodiscard]] QColor getTextColor() const;
    [[nodiscard]] QColor getIconColor() const;
    [[nodiscard]] QColor getIconSeparatorColor() const;
    [[nodiscard]] QColor getCloseButtonIconColor() const;
    [[nodiscard]] QColor getDurationBarColor() const;
    [[nodiscard]] QFont getTitleFont() const;
    [[nodiscard]] QFont getTextFont() const;
    [[nodiscard]] QMargins getMargins() const;
    [[nodiscard]] QMargins getIconMargins() const;
    [[nodiscard]] QMargins getIconSectionMargins() const;
    [[nodiscard]] QMargins getTextSectionMargins() const;
    [[nodiscard]] QMargins getCloseButtonMargins() const;
    [[nodiscard]] int getTextSectionSpacing() const;

    Toast& setDuration(int duration);
    Toast& setShowDurationBar(bool enabled);
    Toast& setTitle(const QString& title);
    Toast& setText(const QString& text);
    Toast& setIcon(const QPixmap& icon);
    Toast& setIcon(const ToastIcon& icon);
    Toast& setShowIcon(bool enabled);
    Toast& setIconSize(const QSize& size);
    Toast& setShowIconSeparator(bool enabled);
    Toast& setIconSeparatorWidth(int width);
    Toast& setCloseButtonIcon(const QPixmap& icon);
    Toast& setCloseButtonIcon(const ToastIcon& icon);
    Toast& setShowCloseButton(bool enabled);
    Toast& setCloseButtonIconSize(const QSize& size);
    Toast& setCloseButtonSize(const QSize& size);
    Toast& setCloseButtonWidth(int width);
    Toast& setCloseButtonHeight(int height);
    Toast& setCloseButtonAlignment(const ToastButtonAlignment& alignment);
    Toast& setFadeInDuration(int duration);
    Toast& setFadeOutDuration(int duration);
    Toast& setResetDurationOnHover(bool enabled);
    Toast& setStayOnTop(bool enabled);
    Toast& setBorderRadius(int borderRadius);
    Toast& setBackgroundColor(const QColor& color);
    Toast& setTitleColor(const QColor& color);
    Toast& setTextColor(const QColor& color);
    Toast& setIconColor(const QColor& color);
    Toast& setIconSeparatorColor(const QColor& color);
    Toast& setCloseButtonIconColor(const QColor& color);
    Toast& setDurationBarColor(const QColor& color);
    Toast& setTitleFont(const QFont& font);
    Toast& setTextFont(const QFont& font);
    Toast& setMargins(const QMargins& margins);
    Toast& setMarginLeft(int margin);
    Toast& setMarginTop(int margin);
    Toast& setMarginRight(int margin);
    Toast& setMarginBottom(int margin);
    Toast& setIconMargins(const QMargins& margins);
    Toast& setIconMarginLeft(int margin);
    Toast& setIconMarginTop(int margin);
    Toast& setIconMarginRight(int margin);
    Toast& setIconMarginBottom(int margin);
    Toast& setIconSectionMargins(const QMargins& margins);
    Toast& setIconSectionMarginLeft(int margin);
    Toast& setIconSectionMarginTop(int margin);
    Toast& setIconSectionMarginRight(int margin);
    Toast& setIconSectionMarginBottom(int margin);
    Toast& setTextSectionMargins(const QMargins& margins);
    Toast& setTextSectionMarginLeft(int margin);
    Toast& setTextSectionMarginTop(int margin);
    Toast& setTextSectionMarginRight(int margin);
    Toast& setTextSectionMarginBottom(int margin);
    Toast& setCloseButtonMargins(const QMargins& margins);
    Toast& setCloseButtonMarginLeft(int margin);
    Toast& setCloseButtonMarginTop(int margin);
    Toast& setCloseButtonMarginRight(int margin);
    Toast& setCloseButtonMarginBottom(int margin);
    Toast& setTextSectionSpacing(int spacing);
    Toast& setFixedSize(const QSize& size);
    Toast& setFixedSize(int width, int height);
    Toast& setFixedWidth(int width);
    Toast& setFixedHeight(int height);
    Toast& applyPreset(const ToastPreset& preset);

public Q_SLOTS:
    void show();
    void hide();

Q_SIGNALS:
    void closed();

protected:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;

private Q_SLOTS:
    void hide_();
    void fadeOut();
    void updateDurationBar();
    void deleteAndShowNextInQueue();

private:
    static constexpr int updatePositionDuration_ = 200;
    static constexpr int durationBarUpdateInterval_ = 5;
    static constexpr int dropShadowSize_ = 5;

    static const QColor successAccentColor_;
    static const QColor warningAccentColor_;
    static const QColor errorAccentColor_;
    static const QColor informationAccentColor_;
    static const QColor defaultAccentColor_;
    static const QColor defaultBackgroundColor_;
    static const QColor defaultTitleColor_;
    static const QColor defaultTextColor_;
    static const QColor defaultIconSeparatorColor_;
    static const QColor defaultCloseButtonIconColor_;
    static const QColor defaultBackgroundColorDark_;
    static const QColor defaultTitleColorDark_;
    static const QColor defaultTextColorDark_;
    static const QColor defaultIconSeparatorColorDark_;
    static const QColor defaultCloseButtonIconColorDark_;

    static const QString TOAST_STYLE;

    int duration_ { 5000 };
    bool showDurationBar_ { true };
    QString title_;
    QString text_;
    QPixmap icon_;
    bool showIcon_ { false };
    QSize iconSize_ { 18, 18 };
    bool showIconSeparator_ { true };
    int iconSeparatorWidth_ { 2 };
    QPixmap closeButtonIcon_;
    bool showCloseButton_ { true };
    QSize closeButtonIconSize_ { 10, 10 };
    QSize closeButtonSize_ { 24, 24 };
    ToastButtonAlignment closeButtonAlignment_ { ToastButtonAlignment::TOP };
    int fadeInDuration_ { 250 };
    int fadeOutDuration_ { 250 };
    bool resetDurationOnHover_ { true };
    bool stayOnTop_ { true };
    int borderRadius_ { 0 };
    QColor backgroundColor_;
    QColor titleColor_;
    QColor textColor_;
    QColor iconColor_;
    QColor iconSeparatorColor_;
    QColor closeButtonIconColor_;
    QColor durationBarColor_;
    QFont titleFont_;
    QFont textFont_;
    QMargins margins_ { 20, 18, 10, 18 };
    QMargins iconMargins_ { 0, 0, 15, 0 };
    QMargins iconSectionMargins_ { 0, 0, 15, 0 };
    QMargins textSectionMargins_ { 0, 0, 15, 0 };
    QMargins closeButtonMargins_ { 0, -8, 0, -8 };
    int textSectionSpacing_ { 8 };

    int elapsedTime_ { 0 };
    bool fadingOut_ { false };
    bool used_ { false };
    QWidget* parent_ { nullptr };

    std::unique_ptr<QLabel> notification_;
    std::unique_ptr<QWidget> dropShadowLayer1_;
    std::unique_ptr<QWidget> dropShadowLayer2_;
    std::unique_ptr<QWidget> dropShadowLayer3_;
    std::unique_ptr<QWidget> dropShadowLayer4_;
    std::unique_ptr<QWidget> dropShadowLayer5_;
    std::unique_ptr<QGraphicsOpacityEffect> opacityEffect_;
    std::unique_ptr<QPushButton> closeButton_;
    std::unique_ptr<QLabel> titleLabel_;
    std::unique_ptr<QLabel> textLabel_;
    std::unique_ptr<QPushButton> iconWidget_;
    std::unique_ptr<QWidget> iconSeparator_;
    std::unique_ptr<QWidget> durationBarContainer_;
    std::unique_ptr<QWidget> durationBar_;
    std::unique_ptr<QWidget> durationBarChunk_;
    std::unique_ptr<QTimer> durationTimer_;
    std::unique_ptr<QTimer> durationBarTimer_;

    void setupUI();
    void updatePositionXY();
    void updatePositionX();
    void updatePositionY();
    void updateStylesheet();
    [[nodiscard]] QPoint calculatePosition() const;
    [[nodiscard]] Toast* getPredecessorToast() const;

    [[nodiscard]] static QString getCurrentDirectory();
    [[nodiscard]] static QImage recolorImage(
        const QImage& image, const QColor& color);
    [[nodiscard]] static QPixmap getIconFromEnum(ToastIcon enumIcon);

    friend class ToastManager;
};

}
