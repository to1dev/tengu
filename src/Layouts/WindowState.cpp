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

#include "WindowState.h"

namespace Daitengu::Layouts {

WindowState::WindowState(QWidget* w)
    : widget_(w)
{
    if (widget_) {
        initialWidth_ = w->width();
        initialHeight_ = w->height();
    }
}

QWidget* WindowState::widget() const
{
    return widget_;
}

WindowConstraints WindowState::constraints() const
{
    return constraints_;
}

void WindowState::setConstraints(const WindowConstraints newConstraints)
{
    constraints_ = newConstraints;
    if (widget_) {
        widget_->setMinimumSize(
            newConstraints.minWidth, newConstraints.minHeight);
        widget_->setMaximumSize(
            newConstraints.maxWidth, newConstraints.maxHeight);
    }
}

void WindowState::setInitialSize(int width, int height)
{
    initialWidth_ = width;
    initialHeight_ = height;
}

int WindowState::initialWidth() const
{
    return initialWidth_;
}

int WindowState::initialHeight() const
{
    return initialHeight_;
}

}
