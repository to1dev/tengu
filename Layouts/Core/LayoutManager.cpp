#include "LayoutManager.h"

namespace Daitengu::Layouts {

LayoutManager::LayoutManager(QApplication* app, QObject* parent)
    : QObject(parent)
    , app_(app)
    , grid_(12, 12)
{
    connect(app, &QApplication::screenAdded, this,
        [this]() { handleScreenChange(); });
    connect(app, &QApplication::screenRemoved, this,
        [this]() { handleScreenChange(); });

    connect(&animations_, &AnimationManager::groupAnimationStarted, this,
        [this]() { Q_EMIT animationStarted(); });
    connect(&animations_, &AnimationManager::groupAnimationFinished, this,
        [this]() { Q_EMIT animationFinished(); });

    updateGridGeometry();
}

LayoutManager::~LayoutManager()
{
    for (auto window : windowConfigs_.keys()) {
        window->removeEventFilter(this);
    }
}

void LayoutManager::registerWindow(QWidget* window, const WindowConfig& config)
{
    if (!window || windowConfigs_.contains(window))
        return;

    windowConfigs_[window] = config;
    setupWindowConnections(window);
    window->installEventFilter(this);

    Q_EMIT windowAdded(window);
}

void LayoutManager::unregisterWindow(QWidget* window)
{
    if (!windowConfigs_.contains(window))
        return;

    window->removeEventFilter(this);
    windowConfigs_.remove(window);
    currentLayout_.remove(window);

    Q_EMIT windowRemoved(window);
}

void LayoutManager::updateWindowConfig(
    QWidget* window, const WindowConfig& config)
{
    if (!windowConfigs_.contains(window))
        return;

    windowConfigs_[window] = config;

    if (currentLayout_.contains(window))
        snapToLayout();

    Q_EMIT windowConfigChanged(window);
}

void LayoutManager::applyLayout(LayoutType type)
{
    auto generator = LayoutFactory::getGenerator(type);
    if (!generator)
        return;

    QMap<QWidget*, GridArea> newLayout = generator(managedWindows());
    if (validateLayout(newLayout)) {
        currentLayout_ = newLayout;
        snapToLayout();
    }
}

void LayoutManager::applyCustomLayout(const QMap<QWidget*, GridArea>& layout)
{
    if (validateLayout(layout)) {
        currentLayout_ = layout;
        snapToLayout();
    }
}

void LayoutManager::snapToLayout()
{
    if (currentLayout_.isEmpty())
        return;

    QMap<QWidget*, QRect> geometries
        = calculateTargetGeometries(currentLayout_);
    for (auto it = geometries.begin(); it != geometries.end(); ++it) {
        if (it.key()) {
            it.key()->setGeometry(it.value());
        }
    }

    Q_EMIT layoutChanged();
}

void LayoutManager::animateToLayout(int duration)
{
    if (currentLayout_.isEmpty())
        return;

    QMap<QWidget*, QRect> geometries
        = calculateTargetGeometries(currentLayout_);
    animations_.startGroupAnimation(geometries, animations_.defaultConfig());
}

void LayoutManager::setGridSize(int rows, int cols)
{
    grid_.setGridSize(rows, cols);
    updateGridGeometry();
    snapToLayout();
}

void LayoutManager::setGridGap(int horizontal, int vertical)
{
    grid_.setGap(horizontal, vertical);
    if (!currentLayout_.isEmpty()) {
        snapToLayout();
    }
}

void LayoutManager::moveToScreen(QScreen* screen)
{
    if (!screen)
        return;

    grid_.setScreenGeometry(screen->availableGeometry());
    snapToLayout();
}

void LayoutManager::distributeAcrossScreens()
{
    QList<QScreen*> screens = app_->screens();
    if (screens.isEmpty() || windowConfigs_.isEmpty())
        return;

    if (screens.size() == 1)
        return;

    QList<QWidget*> windows = windowConfigs_.keys();
    int windowCount = windows.size();
    int screenCount = screens.size();

    int windowPerScreen = windowCount / screenCount;
    int remainingWindows = windowCount % screenCount;

    int currentWindowIndex = 0;
    for (int i = 0; i < screenCount && currentWindowIndex < windowCount; ++i) {
        QScreen* screen = screens[i];
        QRect screenGeometry = screen->availableGeometry();

        int windowsForThisScreen
            = windowPerScreen + (i < remainingWindows ? 1 : 0);

        if (windowsForThisScreen <= 0)
            continue;

        grid_.setScreenGeometry(screenGeometry);

        QMap<QWidget*, GridArea> screenLayout;
        for (int j = 0;
            j < windowsForThisScreen && currentWindowIndex < windowCount; ++j) {
            QWidget* window = windows[currentWindowIndex++];
            if (currentLayout_.contains(window)) {
                screenLayout[window] = currentLayout_[window];
            }
        }

        if (!screenLayout.isEmpty()) {
            QMap<QWidget*, QRect> geometries
                = calculateTargetGeometries(screenLayout);
            for (auto it = geometries.begin(); it != geometries.end(); ++it) {
                it.key()->setGeometry(it.value());
            }
        }
    }

    updateGridGeometry();
}

bool LayoutManager::isAnimating() const
{
    return animations_.isRunning();
}

QList<QWidget*> LayoutManager::managedWindows() const
{
    return windowConfigs_.keys();
}

void LayoutManager::setDefaultAnimationConfig(const AnimationConfig& config)
{
    animations_.setDefaultConfig(config);
}

void LayoutManager::enableAnimations(bool enable)
{
    AnimationConfig config = animations_.defaultConfig();
    config.enabled = enable;
    animations_.setDefaultConfig(config);
}

void LayoutManager::applyGeometryToWindow(QWidget* window, const GridArea& area)
{
    if (!window || !windowConfigs_.contains(window))
        return;

    QRect geometry = grid_.areaToRect(area);
    window->setGeometry(geometry);
}

void LayoutManager::updateGridGeometry()
{
    QScreen* primaryScreen = app_->primaryScreen();
    if (primaryScreen) {
        grid_.setScreenGeometry(primaryScreen->availableGeometry());
    }
}

void LayoutManager::handleScreenChange()
{
    updateGridGeometry();
    snapToLayout();
}

QMap<QWidget*, QRect> LayoutManager::calculateTargetGeometries(
    const QMap<QWidget*, GridArea>& layout)
{
    QMap<QWidget*, QRect> geometries;

    for (auto it = layout.begin(); it != layout.end(); ++it) {
        QWidget* window = it.key();
        const GridArea& area = it.value();

        if (window && windowConfigs_.contains(window)) {
            QRect geometry = grid_.areaToRect(area);
            const WindowConfig& config = windowConfigs_[window];

            if (config.constraints().keepAspectRatio) {
                geometry = GeometryUtils::maintainAspectRatio(geometry,
                    static_cast<double>(window->width()) / window->height());
            }

            geometry = GeometryUtils::constrainRect(
                geometry, grid_.screenGeometry());

            geometries[window] = geometry;
        }
    }

    return geometries;
}

void LayoutManager::setupWindowConnections(QWidget* window)
{
    // TODO: custom signals and slots
}

bool LayoutManager::validateLayout(const QMap<QWidget*, GridArea>& layout) const
{
    for (auto it = layout.begin(); it != layout.end(); ++it) {
        if (!windowConfigs_.contains(it.key())
            || !grid_.isAreaValid(it.value())) {
            return false;
        }
    }

    return true;
}

bool LayoutManager::eventFilter(QObject* watched, QEvent* event)
{
    QWidget* window = qobject_cast<QWidget*>(watched);
    if (!window || !windowConfigs_.contains(window)) {
        return false;
    }

    switch (event->type()) {
    case QEvent::Resize:
    case QEvent::Move: {
        // TODO: resizing and moving
        break;
    }
    default:
        break;
    }

    return false;
}

}
