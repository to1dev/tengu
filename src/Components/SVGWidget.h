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

#ifndef SVGWIDGET_H
#define SVGWIDGET_H

#include <QMouseEvent>
#include <QPainter>
#include <QSvgRenderer>
#include <QWidget>

namespace Daitengu::Components {

class SVGWidget : public QWidget {
    Q_OBJECT

public:
    SVGWidget(const QString& file, QWidget* parent = nullptr, int padding = 0,
        bool clickable = false);

    int padding() const;
    void setPadding(int newPadding);

    bool clickable() const;
    void setClickable(bool newClickable);

    QSize imageSize() const;
    void setImageSize(const QSize &newImageSize);

Q_SIGNALS:
    void clicked();

protected:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QSvgRenderer* renderer_;
    int padding_ { 0 };
    bool clickable_ { false };
    QSize imageSize_;
};

}
#endif // SVGWIDGET_H
