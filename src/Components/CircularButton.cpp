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

#include "CircularButton.h"

#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRegion>

namespace Daitengu::Components {

CircularButton::CircularButton(QWidget* parent)
    : QPushButton(parent)
    , svgRenderer_(nullptr)
    , padding_(6)
    , size_(48)
    , backgroundColor_(Qt::transparent)
    , hoverColor_(QColor(200, 200, 200))
    , isHovered_(false)
{
    updateSize();
    setMouseTracking(true);
    updateRegion();
}

void CircularButton::setSvgIcon(const QString& svgPath)
{
    svgPath_ = svgPath;
    if (svgRenderer_) {
        delete svgRenderer_;
    }

    svgRenderer_ = new QSvgRenderer(svgPath, this);
    update();
}

void CircularButton::setPadding(int padding)
{
    padding_ = padding;
    updateSize();
    update();
}

void CircularButton::setButtonSize(int size)
{
    size_ = size;
    updateSize();
    updateRegion();
    update();
}

void CircularButton::setHoverColor(const QColor& color)
{
    hoverColor_ = color;
    update();
}

void CircularButton::setBackgroundColor(const QColor& color)
{
    backgroundColor_ = color;
    update();
}

void CircularButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QPainterPath path;
    qreal radius = qMin(width(), height()) / 2.0 - 0.5;
    path.addEllipse(QPointF(width() / 2.0, height() / 2.0), radius, radius);

    painter.setPen(Qt::NoPen);
    painter.setBrush(isHovered_ ? hoverColor_ : backgroundColor_);
    painter.drawPath(path);

    if (svgRenderer_ && svgRenderer_->isValid()) {
        int iconSize = width() - 2 * padding_;
        svgRenderer_->render(
            &painter, QRectF(padding_, padding_, iconSize, iconSize));
    }
}

void CircularButton::resizeEvent(QResizeEvent* event)
{
    updateRegion();
}

void CircularButton::enterEvent(QEvent* event)
{
    isHovered_ = true;
    update();
}

void CircularButton::leaveEvent(QEvent* event)
{
    isHovered_ = false;
    update();
}

void CircularButton::updateSize()
{
    int totalSize = size_ + 2 * padding_;
    setFixedSize(totalSize, totalSize);
}

void CircularButton::updateRegion()
{
    QPainterPath path;
    qreal radius = qMin(width(), height()) / 2.0;
    path.addEllipse(QPointF(width() / 2.0, height() / 2.0), radius, radius);
    QRegion region(path.toFillPolygon().toPolygon());
    setMask(region);
}
}
