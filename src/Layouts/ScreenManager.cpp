#include "ScreenManager.h"

namespace Daitengu::Layouts {

ScreenManager::ScreenManager(QObject* parent)
    : QObject(parent)
{
    screens_ = QGuiApplication::screens();

    connect(qApp, &QGuiApplication::screenAdded, this,
        &ScreenManager::handleScreenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this,
        &ScreenManager::handleScreenRemoved);
}

QList<QScreen*> ScreenManager::screens() const
{
    return screens_;
}

void ScreenManager::handleScreenAdded(QScreen* screen)
{
    if (!screens_.contains(screen)) {
        screens_.append(screen);
        Q_EMIT screensChanged();
    }
}

void ScreenManager::handleScreenRemoved(QScreen* screen)
{
    if (screens_.contains(screen)) {
        screens_.removeAll(screen);
        Q_EMIT screensChanged();
    }
}

}
