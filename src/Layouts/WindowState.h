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

#ifndef WINDOWSTATE_H
#define WINDOWSTATE_H

#include <QWidget>

namespace Daitengu::Layouts {

struct WindowConstraints {
    int minWidth { 50 };
    int minHeight { 50 };
    int maxWidth { 10000 };
    int maxHeight { 10000 };
    bool keepAspectRatio { false };
};

class WindowState {
public:
    explicit WindowState(QWidget* w);

    QWidget* widget() const;

    WindowConstraints constraints() const;
    void setConstraints(const WindowConstraints newConstraints);

    void setInitialSize(int width, int height);
    int initialWidth() const;
    int initialHeight() const;

private:
    QWidget* widget_ { nullptr };
    WindowConstraints constraints_;
    int initialWidth_ { 800 };
    int initialHeight_ { 600 };
};

}
#endif // WINDOWSTATE_H
