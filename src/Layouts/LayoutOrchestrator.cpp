#include "LayoutOrchestrator.h"

namespace Daitengu::Layouts {

LayoutOrchestrator::LayoutOrchestrator(QObject* parent)
    : QObject(parent)
    , layoutEngine_(nullptr)
    , animController_(new AnimationController(this))
    , screenManager_(new ScreenManager(this))
{
    connect(screenManager_, &ScreenManager::screensChanged, this,
        &LayoutOrchestrator::onScreensChanged);
}

LayoutOrchestrator::~LayoutOrchestrator()
{
    qDeleteAll(windows_);
    windows_.clear();
}

void LayoutOrchestrator::setLayoutEngine(
    std::unique_ptr<LayoutEngine> newLayoutEngine)
{
    layoutEngine_ = std::move(newLayoutEngine);
}

void LayoutOrchestrator::registerWindow(QWidget* w)
{
    if (!w)
        return;
    for (auto* ws : windows_) {
        if (ws->widget() == w) {
            return;
        }
    }
    auto* state = new WindowState(w);
    windows_.append(state);
}

void LayoutOrchestrator::unregisterWindow(QWidget* w)
{
    for (int i = 0; i < windows_.size(); ++i) {
        if (windows_[i]->widget() == w) {
            delete windows_[i];
            windows_.removeAt(i);
            break;
        }
    }
}

void LayoutOrchestrator::snapToLayout()
{
    if (!layoutEngine_) {
        qWarning() << "[LayoutOrchestrator] No LayoutEngine set!";
        return;
    }
    if (auto* primary = QGuiApplication::primaryScreen()) {
        QRect screenGeom = primary->availableGeometry();
        auto layout = layoutEngine_->calculateLayout(windows_, screenGeom);
        applyLayout(layout);
    }
}

void LayoutOrchestrator::animateToLayout(int duration)
{
    if (!layoutEngine_) {
        qWarning() << "[LayoutOrchestrator] No LayoutEngine set!";
        return;
    }
    if (auto* primary = QGuiApplication::primaryScreen()) {
        QRect screenGeom = primary->availableGeometry();
        auto layout = layoutEngine_->calculateLayout(windows_, screenGeom);
        animController_->animateTo(layout, duration);
    }
}

ScreenManager* LayoutOrchestrator::screenManager() const
{
    return screenManager_;
}

void LayoutOrchestrator::onScreensChanged()
{
    snapToLayout();
}

void LayoutOrchestrator::applyLayout(const QMap<QWidget*, QRect>& layout)
{
    for (auto it = layout.begin(); it != layout.end(); ++it) {
        if (QWidget* w = it.key()) {
            w->setGeometry(it.value());
        }
    }
    Q_EMIT layoutChanged();
}

}
