#ifndef WINDOWCONFIG_H
#define WINDOWCONFIG_H

#include <QUuid>
#include <QWidget>

#include "Types.h"

namespace Daitengu::Layouts {

class WindowConfig {
public:
    WindowConfig();
    explicit WindowConfig(QWidget* window);

    QString id() const;
    QWidget* window() const;
    bool isValid() const;

    void setGridCell(const GridCell& cell);
    void setGridArea(const GridArea& area);
    GridCell gridCell() const;
    GridArea gridArea() const;

    void setConstraints(const WindowConstraints& constraints);
    void setMinSize(int width, int height);
    void setMaxSize(int width, int height);
    void setFixedSize(int width, int height);
    void setAspectRatio(bool keep);
    void setResizable(bool allowResize);
    void setDraggable(bool allowDrag);

    void setAnimationConfig(const AnimationConfig& config);
    void setAnimationDuration(int duration);
    void setAnimationEasing(QEasingCurve::Type easing);
    void enableAnimation(bool enable);

    const WindowConstraints& constraints() const;
    const AnimationConfig& animationConfig() const;

    void applyToWindow();

private:
    QString id_;
    QWidget* window_;
    GridCell cell_;
    GridArea area_;
    WindowConstraints constraints_;
    AnimationConfig animationConfig_;

    void setupWindow();
};

}
#endif // WINDOWCONFIG_H
