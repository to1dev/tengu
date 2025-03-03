#include "WindowManager.h"

namespace Daitengu::Core {

WindowManager::WindowManager()
{
}

WindowManager::~WindowManager()
{
    windows_.clear();
}

void WindowManager::center(QWidget* window)
{
    if (!window)
        return;

    QSize ss = QGuiApplication::primaryScreen()->availableSize();
    window->move((ss.width() - window->frameSize().width()) / 2,
        (ss.height() - window->frameSize().height()) / 2);
}

void WindowManager::reset(QWidget* window, double percent, WindowShape shape)
{
    if (!window)
        return;

    if (percent <= 0.0 || percent > 1.0) {
        qWarning() << "Invalid percent value: " << percent
                   << ". Resetting to default.";
        percent = 0.8;
    }

    QRect screenGeometry
        = QGuiApplication::primaryScreen()->availableGeometry();

    int targetWidth = DEFAULT_WINDOW_WIDTH;
    int targetHeight = DEFAULT_WINDOW_HEIGHT;

    switch (shape) {
    case WindowShape::TOPBAR: {
        int currentHeight = window->height();

        window->setGeometry(screenGeometry.x(), screenGeometry.y(),
            screenGeometry.width(), currentHeight);

        return;
    }

    case WindowShape::LEFT_PANEL: {
        return;
    }

    case WindowShape::RIGHT_PANEL: {
        if (windows_.contains(WindowShape::TOPBAR)
            && windows_[WindowShape::TOPBAR]) {
            int topHeight = windows_[WindowShape::TOPBAR]->height();
            int panelWidth = window->width();

            QRect panelGeometry(screenGeometry.right() - panelWidth,
                screenGeometry.top() + topHeight, panelWidth,
                screenGeometry.height() - topHeight);

            window->setGeometry(panelGeometry);
        }

        return;
    }

    case WindowShape::HORIZONTAL: {
        if (screenGeometry.width() > screenGeometry.height()) {
            targetHeight = screenGeometry.height() * percent;

            double aspectRatio = screenGeometry.width()
                / static_cast<double>(screenGeometry.height());
            targetWidth = (aspectRatio > GOLDEN_RATIO)
                ? targetHeight * GOLDEN_RATIO
                : targetHeight * aspectRatio;
        } else {
            targetWidth = screenGeometry.width() * percent;
            targetHeight = targetWidth / GOLDEN_RATIO;
        }

        break;
    }

    case WindowShape::SQUARE: {
        int baseLength
            = std::min(screenGeometry.width(), screenGeometry.height())
            * percent;
        targetWidth = targetHeight = baseLength;

        break;
    }

    default:
        qWarning() << "Unsupported WindowShape encountered!";
        break;
    }

    targetWidth = std::max(targetWidth, DEFAULT_WINDOW_WIDTH);
    targetHeight = std::max(targetHeight, DEFAULT_WINDOW_HEIGHT);

    window->resize(targetWidth, targetHeight);
    center(window);
}

void WindowManager::addWindow(const WindowShape& shape, QWidget* window)
{
    if (window) {
        switch (shape) {
        case WindowShape::TOPBAR:
        case WindowShape::LEFT_PANEL:
        case WindowShape::RIGHT_PANEL:
            windows_[shape] = window;
            break;

        default:
            break;
        }
    }
}

}
