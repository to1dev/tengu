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
    , window_(parent)
{
}

bool TitleBar::fixed() const
{
    return fixed_;
}

void TitleBar::setFixed(bool newFixed)
{
    fixed_ = newFixed;
}

void TitleBar::mousePressEvent(QMouseEvent* event)
{
    if (Qt::LeftButton == event->buttons()) {
        pressed_ = true;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        pos_ = event->globalPos();
#else
        pos_ = event->globalPosition().toPoint();
#endif
        oldPos_ = window_->pos();
    }
}

void TitleBar::mouseMoveEvent(QMouseEvent* event)
{
    if (!window_->isMaximized() && pressed_
        && Qt::LeftButton == event->buttons()) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QPoint newPos = oldPos_ + (event->globalPos() - pos_);
#else
        QPoint newPos = mOldPos + (event->globalPosition().toPoint() - mPos);
#endif

        if (fixed_) {
            QRect virtualRect = desktopRect();

            if (newPos.x() < virtualRect.left())
                newPos.setX(virtualRect.left());
            if (newPos.y() < virtualRect.top())
                newPos.setY(virtualRect.top());
            int dx = virtualRect.width() - window_->frameSize().width();
            int dy = virtualRect.height() - window_->frameSize().height();

            if (newPos.x() > dx)
                newPos.setX(dx);
            if (newPos.y() > dy)
                newPos.setY(dy);
        }

        window_->move(newPos);
    }
}

void TitleBar::mouseReleaseEvent(QMouseEvent* event)
{
    (void)event;
    if (Qt::LeftButton == event->buttons()) {
        pressed_ = false;
    }
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_EMIT doubleClick();

    QWidget::mouseDoubleClickEvent(event);
}

}
