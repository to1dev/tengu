// SPDX-License-Identifier: AGPL-3.0-or-later
/*
 * Copyright (C) 2025 to1dev <https://arc20.me/to1dev>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

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

        if (snapped_) {
            QSize windowSize = window_->frameSize();
            QRect windowRect(newPos, windowSize);

            QRect screenRect = desktopRect();
            newPos = snapToScreenEdges(newPos, windowRect, screenRect);
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

QPoint TitleBar::snapToScreenEdges(
    const QPoint& pos, const QRect& windowRect, const QRect& screenRect)
{
    QPoint newPos = pos;

    if (qAbs(pos.x() - screenRect.left()) < SNAP_DISTANCE) {
        newPos.setX(screenRect.left());
    }

    int rightPos = screenRect.right() - windowRect.width();
    if (qAbs(pos.x() - rightPos) < SNAP_DISTANCE) {
        newPos.setX(rightPos);
    }

    if (qAbs(pos.y() - screenRect.top()) < SNAP_DISTANCE) {
        newPos.setY(screenRect.top());
    }

    int bottomPos = screenRect.bottom() - windowRect.height();
    if (qAbs(pos.y() - bottomPos) < SNAP_DISTANCE) {
        newPos.setY(bottomPos);
    }

    int screenCenterX = screenRect.left() + screenRect.width() / 2;
    int windowCenterX = pos.x() + windowRect.width() / 2;

    if (qAbs(windowCenterX - screenCenterX) < SNAP_DISTANCE) {
        newPos.setX(screenCenterX - windowRect.width() / 2);
    }

    int screenCenterY = screenRect.top() + screenRect.height() / 2;
    int windowCenterY = pos.y() + windowRect.height() / 2;

    if (qAbs(windowCenterY - screenCenterY) < SNAP_DISTANCE) {
        newPos.setY(screenCenterY - windowRect.height() / 2);
    }

    return newPos;
}

}
