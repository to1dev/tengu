#include "WindowConfig.h"

namespace Daitengu::Layouts {

WindowConfig::WindowConfig()
    : id_(QUuid::createUuid().toString())
    , window_(nullptr)
{
}

WindowConfig::WindowConfig(QWidget* window)
    : id_(QUuid::createUuid().toString())
    , window_(window)
{
    if (window) {
        setupWindow();
    }
}

QString WindowConfig::id() const
{
    return id_;
}

QWidget* WindowConfig::window() const
{
    return window_;
}

bool WindowConfig::isValid() const
{
    return window_ != nullptr;
}

void WindowConfig::setGridCell(const GridCell& cell)
{
    cell_ = cell;
}

void WindowConfig::setGridArea(const GridArea& area)
{
    area_ = area;
}

GridCell WindowConfig::gridCell() const
{
    return cell_;
}

GridArea WindowConfig::gridArea() const
{
    return area_;
}

void WindowConfig::setConstraints(const WindowConstraints& constraints)
{
    constraints_ = constraints;
    if (window_) {
        applyToWindow();
    }
}

void WindowConfig::setMinSize(int width, int height)
{
    constraints_.minWidth = width;
    constraints_.minHeight = height;
    if (window_) {
        window_->setMinimumSize(width, height);
    }
}

void WindowConfig::setMaxSize(int width, int height)
{
    constraints_.maxWidth = width;
    constraints_.maxHeight = height;
    if (window_) {
        window_->setMaximumSize(width, height);
    }
}

void WindowConfig::setFixedSize(int width, int height)
{
    setMinSize(width, height);
    setMaxSize(width, height);
    setResizable(false);
}

void WindowConfig::setAspectRatio(bool keep)
{
    constraints_.keepAspectRatio = keep;
    if (window_ && keep) {
        window_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    }
}

void WindowConfig::setResizable(bool allowResize)
{
    constraints_.allowResize = allowResize;
    if (window_) {
        Qt::WindowFlags flags = window_->windowFlags();
        if (!allowResize) {
            flags |= Qt::MSWindowsFixedSizeDialogHint;
        } else {
            flags &= ~Qt::MSWindowsFixedSizeDialogHint;
        }
        window_->setWindowFlags(flags);
    }
}

void WindowConfig::setDraggable(bool allowDrag)
{
    constraints_.allowDrag = allowDrag;
    if (window_) {
        Qt::WindowFlags flags = window_->windowFlags();
        if (!allowDrag) {
            flags &= ~Qt::WindowFlags(
                Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
        } else {
            flags |= Qt::WindowFlags(
                Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
        }
        window_->setWindowFlags(flags);
    }
}

void WindowConfig::setAnimationConfig(const AnimationConfig& config)
{
    animationConfig_ = config;
}

void WindowConfig::setAnimationDuration(int duration)
{
    animationConfig_.duration = duration;
}

void WindowConfig::setAnimationEasing(QEasingCurve::Type easing)
{
    animationConfig_.easing = easing;
}

void WindowConfig::enableAnimation(bool enable)
{
    animationConfig_.enabled = enable;
}

const WindowConstraints& WindowConfig::constraints() const
{
    return constraints_;
}

const AnimationConfig& WindowConfig::animationConfig() const
{
    return animationConfig_;
}

void WindowConfig::applyToWindow()
{
    if (!window_)
        return;

    window_->setMinimumSize(constraints_.minWidth, constraints_.minHeight);
    window_->setMaximumSize(constraints_.maxWidth, constraints_.maxHeight);

    Qt::WindowFlags flags = window_->windowFlags();

    if (!constraints_.allowResize) {
        flags |= Qt::MSWindowsFixedSizeDialogHint;
    }

    if (!constraints_.allowDrag) {
        flags
            &= ~Qt::WindowFlags(Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
    }

    window_->setWindowFlags(flags);

    if (constraints_.keepAspectRatio) {
        window_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    }
}

void WindowConfig::setupWindow()
{
    applyToWindow();

    if (window_->size().isValid()) { }
}

}
