#pragma once

#include <algorithm>
#include <deque>
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
    ERROR,
    INFORMATION,
    SUCCESS_DARK,
    WARNING_DARK,
    ERROR_DARK,
    INFORMATION_DARK
};

enum class ToastIcon {
    SUCCESS,
    WARNING,
    ERROR,
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
    static void setPosition(ToastPosition position);
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

}
