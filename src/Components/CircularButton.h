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

#pragma once

#include <QPushButton>
#include <QSvgRenderer>
#include <QWidget>

namespace Daitengu::Components {

class CircularButton : public QPushButton {
    Q_OBJECT

public:
    explicit CircularButton(QWidget* parent = nullptr);

    void setSvgIcon(const QString& svgPath);

    void setPadding(int padding);

    void setButtonSize(int size);

    void setHoverColor(const QColor& color);

    void setBackgroundColor(const QColor& color);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void updateSize();
    void updateRegion();

    QSvgRenderer* svgRenderer_;
    QString svgPath_;
    int padding_;
    int size_;
    QColor backgroundColor_;
    QColor hoverColor_;
    bool isHovered_;
};
}
