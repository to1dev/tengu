#include "WindowState.h"

namespace Daitengu::Layouts {

WindowState::WindowState(QWidget* w)
    : widget_(w)
{
    if (widget_) {
        initialWidth_ = w->width();
        initialHeight_ = w->height();
    }
}

QWidget* WindowState::widget() const
{
    return widget_;
}

WindowConstraints WindowState::constraints() const
{
    return constraints_;
}

void WindowState::setConstraints(const WindowConstraints newConstraints)
{
    constraints_ = newConstraints;
    if (widget_) {
        widget_->setMinimumSize(
            newConstraints.minWidth, newConstraints.minHeight);
        widget_->setMaximumSize(
            newConstraints.maxWidth, newConstraints.maxHeight);
    }
}

void WindowState::setInitialSize(int width, int height)
{
    initialWidth_ = width;
    initialHeight_ = height;
}

int WindowState::initialWidth() const
{
    return initialWidth_;
}

int WindowState::initialHeight() const
{
    return initialHeight_;
}

}
