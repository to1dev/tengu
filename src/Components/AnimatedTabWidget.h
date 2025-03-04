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

#ifndef ANIMATEDTABWIDGET_H
#define ANIMATEDTABWIDGET_H

#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QRect>
#include <QTabBar>
#include <QTabWidget>

namespace Daitengu::Components {

class AnimatedTabWidget : public QTabWidget {
    Q_OBJECT

public:
    enum class AnimationType {
        Fade,
        SlideHorizontal,
        SlideVertical,
        SlideFade,
    };

    explicit AnimatedTabWidget(QWidget* parent = nullptr);

    void setAnimationDuration(int newAnimationDuration);

    void setAnimationType(AnimationType newAnimationType);

    void setCurrentIndexEx(int index);

private Q_SLOTS:
    void onTabBarClicked(int index);

private:
    void animateFade(int index);
    void animateSlide(int index, bool horizontal);
    void animateSlideFade(int index, bool horizontal);

    int animationDuration_;
    AnimationType animationType_;
};

}
#endif // ANIMATEDTABWIDGET_H
