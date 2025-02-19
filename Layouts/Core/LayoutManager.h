#ifndef LAYOUTMANAGER_H
#define LAYOUTMANAGER_H

#include <QEvent>
#include <QMap>
#include <QMoveEvent>
#include <QObject>
#include <QResizeEvent>
#include <QScreen>

#include "../Utils/GeometryUtils.h"
#include "AnimationManager.h"
#include "GridSystem.h"
#include "LayoutFactory.h"
#include "WindowConfig.h"

namespace Daitengu::Layouts {

class LayoutManager : public QObject {
    Q_OBJECT

public:
    explicit LayoutManager(QApplication* app);
    ~LayoutManager();

    void registerWindow(
        QWidget* window, const WindowConfig& config = WindowConfig());
    void unregisterWindow(QWidget* window);
    void updateWindowConfig(QWidget* window, const WindowConfig& config);

    void applyLayout(LayoutType type);
    void applyCustomLayout(const QMap<QWidget*, GridArea>& layout);
    void snapToLayout();
    void animateToLayout(int duration = 300);

    void setGridSize(int rows, int cols);
    void setGridGap(int horizontal, int vertical);

    void moveToScreen(QScreen* screen);
    void distributeAcrossScreens();

    bool isAnimating() const;
    QList<QWidget*> managedWindows() const;

    void setDefaultAnimationConfig(const AnimationConfig& config);
    void enableAnimations(bool enable);

Q_SIGNALS:
    void layoutChanged();
    void windowAdded(QWidget* window);
    void windowRemoved(QWidget* window);
    void animationStarted();
    void animationFinished();

private:
    QApplication* app_;
    GridSystem grid_;
    AnimationManager animations_;
    QMap<QWidget*, WindowConfig> windowConfigs_;
    QMap<QWidget*, GridArea> currentLayout_;

    void applyGeometryToWindow(QWidget* window, const GridArea& area);
    void updateGridGeometry();
    void handleScreenChange();
    QMap<QWidget*, QRect> calculateTargetGeometries(
        const QMap<QWidget*, GridArea>& layout);
    void setupWindowConnections(QWidget* window);
    bool validateLayout(const QMap<QWidget*, GridArea>& layout) const;

    bool eventFilter(QObject* watched, QEvent* event) override;
};

}
#endif // LAYOUTMANAGER_H
