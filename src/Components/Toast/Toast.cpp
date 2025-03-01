#include "Toast.h"

namespace Daitengu::Components {

constinit int ToastManager::maximumOnScreen_ = 3;
constinit int ToastManager::spacing_ = 10;
constinit int ToastManager::offsetX_ = 20;
constinit int ToastManager::offsetY_ = 45;
constinit bool ToastManager::alwaysOnMainScreen_ = false;
std::optional<QScreen*> ToastManager::fixedScreen_ = std::nullopt;
constinit ToastPosition ToastManager::position_ = ToastPosition::BOTTOM_RIGHT;
std::deque<Toast*> ToastManager::currentlyShown_;
std::deque<Toast*> ToastManager::queue_;
std::mutex ToastManager::mutex_;

int ToastManager::getMaximumOnScreen()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return maximumOnScreen_;
}

int ToastManager::getSpacing()
{
    return 0;
}
QPoint ToastManager::getOffset()
{
    return QPoint();
}
int ToastManager::getOffsetX()
{
    return 0;
}
int ToastManager::getOffsetY()
{
    return 0;
}
bool ToastManager::isAlwaysOnMainScreen()
{
    return false;
}
std::optional<std::reference_wrapper<QScreen>> ToastManager::getFixedScreen()
{
    return std::optional<std::reference_wrapper<QScreen>>();
}
ToastPosition ToastManager::getPosition()
{
    return ToastPosition();
}
int ToastManager::getCount()
{
    return 0;
}
int ToastManager::getVisibleCount()
{
    return 0;
}
int ToastManager::getQueuedCount()
{
    return 0;
}
void ToastManager::setMaximumOnScreen(int maximum)
{
}
void ToastManager::setSpacing(int spacing)
{
}
void ToastManager::setOffset(int x, int y)
{
}
void ToastManager::setOffsetX(int offsetX)
{
}
void ToastManager::setOffsetY(int offsetY)
{
}
void ToastManager::setAlwaysOnMainScreen(bool enabled)
{
}
void ToastManager::setFixedScreen(QScreen* screen)
{
}
void ToastManager::setPosition(ToastPosition position)
{
}
void ToastManager::reset()
{
}
void ToastManager::addToast(Toast* toast)
{
}
void ToastManager::removeToast(Toast* toast)
{
}
void ToastManager::showNextInQueue()
{
}
void ToastManager::updateCurrentlyShowingPositionXY()
{
}
void ToastManager::updateCurrentlyShowingPositionX()
{
}
void ToastManager::updateCurrentlyShowingPositionY()
{
}
}
