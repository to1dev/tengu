#ifndef LAYOUTMANAGER_H
#define LAYOUTMANAGER_H

#include <QApplication>
#include <QDebug>
#include <QScreen>
#include <QWidget>

namespace Daitengu::Core {

inline constexpr int DEFAULT_WINDOW_WIDTH = 1024;
inline constexpr int DEFAULT_WINDOW_HEIGHT = 633;
inline constexpr int WM_MARGIN = 5;
inline constexpr double GOLDEN_RATIO = 1.618;
inline constexpr double HORIZONTAL_RATIO = 0.8;

enum class WindowShape {
    HORIZONTAL = 0,
    VERTICAL,
    SQUARE,
};

class LayoutManager {
public:
    LayoutManager(QApplication* app = nullptr);
    ~LayoutManager();

    void center();
    void reset(double percent = HORIZONTAL_RATIO,
        WindowShape shape = WindowShape::HORIZONTAL);

    QWidget* window() const;
    void setWindow(QWidget* newWindow);

private:
    QApplication* app_;
    QWidget* window_ { nullptr };
    double ratio_ { 1.0 };
};

}
#endif // LAYOUTMANAGER_H
