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

#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QApplication>
#include <QMouseEvent>
#include <QScreen>
#include <QWidget>

namespace Daitengu::Components {

static constexpr int SNAP_DISTANCE = 15;

class TitleBar : public QWidget {
    Q_OBJECT

public:
    TitleBar(QWidget* parent = nullptr);

    bool fixed() const;
    void setFixed(bool newFixed);

Q_SIGNALS:
    void doubleClick();

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    QPoint snapToScreenEdges(
        const QPoint& pos, const QRect& windowRect, const QRect& screenRect);

    QWidget* window_ { nullptr };
    bool pressed_ { false };
    bool fixed_ { false };
    bool snapped_ { false };
    QPoint pos_;
    QPoint oldPos_;
};

}
#endif // TITLEBAR_H
