#include "TitleBar.h"

namespace Daitengu::Components {

inline QRect desktopRect()
{
    QRegion virtualGeometry;
    for (auto screen : QGuiApplication::screens()) {
        virtualGeometry += screen->availableGeometry();
    }
    return virtualGeometry.boundingRect();
}

TitleBar::TitleBar(QWidget* parent)
    : QWidget(parent)
    , mWindow(parent)
{
}

bool TitleBar::fixed() const
{
    return mFixed;
}

void TitleBar::setFixed(bool newFixed)
{
    mFixed = newFixed;
}

void TitleBar::mousePressEvent(QMouseEvent* event)
{
    if (Qt::LeftButton == event->buttons()) {
        mPressed = true;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        mPos = event->globalPos();
#else
        mPos = event->globalPosition().toPoint();
#endif
        mOldPos = mWindow->pos();
    }
}

void TitleBar::mouseMoveEvent(QMouseEvent* event)
{
    if (!mWindow->isMaximized() && mPressed
        && Qt::LeftButton == event->buttons()) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QPoint newPos = mOldPos + (event->globalPos() - mPos);
#else
        QPoint newPos = mOldPos + (event->globalPosition().toPoint() - mPos);
#endif

        if (mFixed) {
            QRect virtualRect = desktopRect();

            if (newPos.x() < virtualRect.left())
                newPos.setX(virtualRect.left());
            if (newPos.y() < virtualRect.top())
                newPos.setY(virtualRect.top());
            int dx = virtualRect.width() - mWindow->frameSize().width();
            int dy = virtualRect.height() - mWindow->frameSize().height();

            if (newPos.x() > dx)
                newPos.setX(dx);
            if (newPos.y() > dy)
                newPos.setY(dy);
        }

        mWindow->move(newPos);
    }
}

void TitleBar::mouseReleaseEvent(QMouseEvent* event)
{
    (void)event;
    if (Qt::LeftButton == event->buttons()) {
        mPressed = false;
    }
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    emit doubleClick();

    QWidget::mouseDoubleClickEvent(event);
}

}
