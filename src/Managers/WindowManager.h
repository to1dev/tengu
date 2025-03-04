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

#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <unordered_map>

#include <QApplication>
#include <QDebug>
#include <QScreen>
#include <QWidget>

namespace Daitengu::Core {

inline constexpr int DEFAULT_WINDOW_WIDTH = 800;
inline constexpr int DEFAULT_WINDOW_HEIGHT = 494;
inline constexpr int WM_MARGIN = 5;
inline constexpr double GOLDEN_RATIO = 1.618;
inline constexpr double HORIZONTAL_RATIO = 0.8;

class WindowManager {
public:
    enum class WindowShape {
        HORIZONTAL = 0,
        VERTICAL,
        SQUARE,
        TOPBAR,
        LEFT_PANEL,
        RIGHT_PANEL,
    };

    WindowManager();
    ~WindowManager();

    void center(QWidget* window = nullptr);
    void reset(QWidget* window = nullptr, double percent = HORIZONTAL_RATIO,
        WindowShape shape = WindowShape::HORIZONTAL);

    void addWindow(const WindowShape& shape, QWidget* window);

private:
    double ratio_ { 1.0 };

    std::unordered_map<WindowShape, QWidget*> windows_;
};

}
#endif // WINDOWMANAGER_H
