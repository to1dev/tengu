#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <unordered_map>

#include <QApplication>
#include <QDebug>
#include <QScreen>
#include <QWidget>

namespace Daitengu::Core {

inline constexpr int DEFAULT_WINDOW_WIDTH = 800;
inline constexpr int DEFAULT_WINDOW_HEIGHT = 494;
inline constexpr int WM_MARGIN = 5;
inline constexpr double GOLDEN_RATIO = 1.618;
inline constexpr double HORIZONTAL_RATIO = 0.8;

enum class WindowShape {
    HORIZONTAL = 0,
    VERTICAL,
    SQUARE,
    TOPBAR,
    LEFT_PANEL,
    RIGHT_PANEL,
};

class WindowManager {
public:
    WindowManager();
    ~WindowManager();

    void center(QWidget* window = nullptr);
    void reset(QWidget* window = nullptr, double percent = HORIZONTAL_RATIO,
        WindowShape shape = WindowShape::HORIZONTAL);

    void addWindow(QWidget* window = nullptr);

private:
    double ratio_ { 1.0 };

    std::unordered_map<QString, QWidget*> windows_;
};

}
#endif // WINDOWMANAGER_H
