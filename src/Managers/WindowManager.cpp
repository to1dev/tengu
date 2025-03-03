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

    QSize screenSize = QGuiApplication::primaryScreen()->availableSize();

    int targetWidth = DEFAULT_WINDOW_WIDTH;
    int targetHeight = DEFAULT_WINDOW_HEIGHT;

    switch (shape) {
    case WindowShape::HORIZONTAL: {
        if (screenSize.width() > screenSize.height()) {
            targetHeight = screenSize.height() * percent;

            double aspectRatio
                = screenSize.width() / static_cast<double>(screenSize.height());
            targetWidth = (aspectRatio > GOLDEN_RATIO)
                ? targetHeight * GOLDEN_RATIO
                : targetHeight * aspectRatio;
        } else {
            targetWidth = screenSize.width() * percent;
            targetHeight = targetWidth / GOLDEN_RATIO;
        }

        break;
    }

    case WindowShape::SQUARE: {
        int baseLength
            = std::min(screenSize.width(), screenSize.height()) * percent;
        targetWidth = targetHeight = baseLength;

        break;
    }

    case WindowShape::VERTICAL: {
        break;
    }

    default: {
        qWarning() << "Unsupported WindowShape encountered!";
        break;
    }
    }

    targetWidth = std::max(targetWidth, DEFAULT_WINDOW_WIDTH);
    targetHeight = std::max(targetHeight, DEFAULT_WINDOW_HEIGHT);

    window->resize(targetWidth, targetHeight);
    center(window);
}

void WindowManager::addWindow(QWidget* window)
{
    if (window) {
        QString name = window->objectName();
        if (name.isEmpty()) {
            qWarning() << "Window has no name, consider setting objectName";
            return;
        }
        windows_[name] = window;
    }
}

}
