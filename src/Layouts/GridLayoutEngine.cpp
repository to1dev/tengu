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

#include "GridLayoutEngine.h"

namespace Daitengu::Layouts {

GridLayoutEngine::GridLayoutEngine(
    int rows, int cols, int hGap, int vGap, const QMargins& padding)
    : rows_(rows)
    , cols_(cols)
    , hGap_(hGap)
    , vGap_(vGap)
    , padding_(padding)
{
}

QMap<QWidget*, QRect> GridLayoutEngine::calculateLayout(
    const QList<WindowState*>& windows, const QRect& screenGeometry)
{
    QMap<QWidget*, QRect> result;

    if (rows_ <= 0 || cols_ <= 0 || screenGeometry.isEmpty()
        || windows.isEmpty()) {
        for (auto* wstate : windows) {
            if (auto* w = wstate->widget()) {
                result[w] = QRect(screenGeometry.topLeft(), w->size());
            }
        }
        return result;
    }

    QRect adjustedScreen = screenGeometry.adjusted(
        padding_.left(), padding_.top(), -padding_.right(), -padding_.bottom());

    double cellWidth = (adjustedScreen.width() - (cols_ - 1) * hGap_)
        / static_cast<double>(cols_);
    double cellHeight = (adjustedScreen.height() - (rows_ - 1) * vGap_)
        / static_cast<double>(rows_);

    int index = 0;
    for (auto* wstate : windows) {
        int row = index / cols_;
        int col = index % cols_;
        if (row >= rows_) {
            break;
        }

        int x = adjustedScreen.x() + col * (cellWidth + hGap_);
        int y = adjustedScreen.y() + row * (cellHeight + vGap_);
        int w = static_cast<int>(cellWidth);
        int h = static_cast<int>(cellHeight);

        QRect target(x, y, w, h);

        const auto& c = wstate->constraints();
        QSize constrainedSize = target.size();
        constrainedSize.setWidth(
            std::clamp(constrainedSize.width(), c.minWidth, c.maxWidth));
        constrainedSize.setHeight(
            std::clamp(constrainedSize.height(), c.minHeight, c.maxHeight));
        target.setSize(constrainedSize);

        if (c.keepAspectRatio) {
            double desiredAspect = static_cast<double>(wstate->initialWidth())
                / static_cast<double>(wstate->initialHeight());
            if (desiredAspect > 0.0) {
                double actualAspect
                    = static_cast<double>(target.width()) / target.height();
                if (actualAspect > desiredAspect) {
                    int newWidth
                        = static_cast<int>(target.height() * desiredAspect);
                    target.setX(target.x() + (target.width() - newWidth) / 2);
                    target.setWidth(newWidth);
                } else {
                    int newHeight
                        = static_cast<int>(target.width() / desiredAspect);
                    target.setY(target.y() + (target.height() - newHeight) / 2);
                    target.setHeight(newHeight);
                }
            }
        }

        if (auto* wdg = wstate->widget()) {
            result[wdg] = target;
        }
        ++index;
    }

    return result;
}

}
