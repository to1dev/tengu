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

#ifndef FLOWLAYOUT_H
#define FLOWLAYOUT_H

#include <array>

#include <QGraphicsLayout>
#include <QRect>
#include <QRectF>
#include <QVector>
#include <QtMath>

namespace Daitengu::Components {

class FlowLayout : public QGraphicsLayout {
public:
    explicit FlowLayout(QGraphicsLayoutItem* parent = nullptr);
    inline void addItem(QGraphicsLayoutItem* item);
    void insertItem(int index, QGraphicsLayoutItem* item);
    void setSpacing(Qt::Orientations o, qreal spacing);
    qreal spacing(Qt::Orientation o) const;

    void setGeometry(const QRectF& geom) override;

    [[nodiscard]] int count() const override;
    QGraphicsLayoutItem* itemAt(int index) const override;
    void removeAt(int index) override;

protected:
    QSizeF sizeHint(
        Qt::SizeHint which, const QSizeF& constraint = QSizeF()) const override;

private:
    qreal doLayout(const QRectF& geom, bool applyNewGeometry) const;
    QSizeF minSize(const QSizeF& constraint) const;
    QSizeF prefSize() const;
    QSizeF maxSize() const;

    QVector<QGraphicsLayoutItem*> items_;
    std::array<qreal, 2> spacing_ { { 6, 6 } };
};

inline void FlowLayout::addItem(QGraphicsLayoutItem* item)
{
    insertItem(-1, item);
}

}
#endif // FLOWLAYOUT_H
