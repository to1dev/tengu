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

#include "AnimatedTabWidget.h"

namespace Daitengu::Components {

AnimatedTabWidget::AnimatedTabWidget(QWidget* parent)
    : QTabWidget(parent)
    , animationDuration_(300)
    , animationType_(AnimationType::Fade)
{
    QTabBar* _tabBar = tabBar();
    if (_tabBar)
        connect(_tabBar, &QTabBar::tabBarClicked, this,
            &AnimatedTabWidget::onTabBarClicked);
}

void AnimatedTabWidget::setAnimationDuration(int newAnimationDuration)
{
    animationDuration_ = newAnimationDuration;
}

void AnimatedTabWidget::setAnimationType(AnimationType newAnimationType)
{
    animationType_ = newAnimationType;
}

void AnimatedTabWidget::setCurrentIndexEx(int index)
{
    if (index == currentIndex() || index < 0 || index >= count()) {
        QTabWidget::setCurrentIndex(index);
        return;
    }

    switch (animationType_) {
    case AnimationType::Fade: {
        animateFade(index);
        break;
    }

    case AnimationType::SlideHorizontal: {
        animateSlide(index, true);
        break;
    }

    case AnimationType::SlideVertical: {
        animateSlide(index, false);
        break;
    }

    case AnimationType::SlideFade: {
        animateSlideFade(index, true);
        break;
    }

    default:
        QTabWidget::setCurrentIndex(index);
        break;
    }
}

void AnimatedTabWidget::onTabBarClicked(int index)
{
    if (index == currentIndex() || index < 0 || index >= count())
        return;

    switch (animationType_) {
    case AnimationType::Fade: {
        animateFade(index);
        break;
    }

    case AnimationType::SlideHorizontal: {
        animateSlide(index, true);
        break;
    }

    case AnimationType::SlideVertical: {
        animateSlide(index, false);
        break;
    }

    case AnimationType::SlideFade: {
        animateSlideFade(index, true);
        break;
    }

    default:
        break;
    }
}

void AnimatedTabWidget::animateFade(int index)
{
    QWidget* oldWidget = currentWidget();
    if (!oldWidget)
        return;

    QGraphicsOpacityEffect* fadeEffect = new QGraphicsOpacityEffect(oldWidget);
    oldWidget->setGraphicsEffect(fadeEffect);

    QPropertyAnimation* fadeOut = new QPropertyAnimation(fadeEffect, "opacity");
    fadeOut->setDuration(animationDuration_);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);

    connect(fadeOut, &QPropertyAnimation::finished, this, [=, this]() {
        oldWidget->setGraphicsEffect(nullptr);
        fadeEffect->deleteLater();
        setCurrentIndex(index);

        QWidget* newWidget = currentWidget();
        if (!newWidget)
            return;

        QGraphicsOpacityEffect* fadeInEffect
            = new QGraphicsOpacityEffect(newWidget);
        newWidget->setGraphicsEffect(fadeInEffect);

        QPropertyAnimation* fadeIn
            = new QPropertyAnimation(fadeInEffect, "opacity");
        fadeIn->setDuration(animationDuration_);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);

        connect(fadeIn, &QPropertyAnimation::finished, this, [=]() {
            newWidget->setGraphicsEffect(nullptr);
            fadeInEffect->deleteLater();
            fadeIn->deleteLater();
        });

        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
        fadeOut->deleteLater();
    });

    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimatedTabWidget::animateSlide(int index, bool horizontal)
{
    QWidget* oldWidget = currentWidget();
    QWidget* newWidget = widget(index);
    if (!oldWidget || !newWidget)
        return;

    QRect rect = oldWidget->geometry();
    int offset = horizontal ? rect.width() : rect.height();
    int direction = (index > currentIndex()) ? 1 : -1;

    QRect oldStart = rect;
    QRect oldEnd = rect;
    if (horizontal) {
        oldEnd.translate(-direction * offset, 0);
    } else {
        oldEnd.translate(0, -direction * offset);
    }

    QRect newStart = rect;
    if (horizontal) {
        newStart.translate(direction * offset, 0);
    } else {
        newStart.translate(0, direction * offset);
    }
    QRect newEnd = rect;

    newWidget->setGeometry(newStart);
    newWidget->show();
    newWidget->raise();

    QPropertyAnimation* oldAnim = new QPropertyAnimation(oldWidget, "geometry");
    oldAnim->setDuration(animationDuration_);
    oldAnim->setStartValue(oldStart);
    oldAnim->setEndValue(oldEnd);

    QPropertyAnimation* newAnim = new QPropertyAnimation(newWidget, "geometry");
    newAnim->setDuration(animationDuration_);
    newAnim->setStartValue(newStart);
    newAnim->setEndValue(newEnd);

    QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
    group->addAnimation(oldAnim);
    group->addAnimation(newAnim);

    connect(group, &QParallelAnimationGroup::finished, this, [=, this]() {
        setCurrentIndex(index);
        oldWidget->setGeometry(rect);
        newWidget->setGeometry(rect);
        group->deleteLater();
    });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimatedTabWidget::animateSlideFade(int index, bool horizontal)
{
    QWidget* oldWidget = currentWidget();
    QWidget* newWidget = widget(index);
    if (!oldWidget || !newWidget)
        return;

    QRect rect = oldWidget->geometry();
    int offset = horizontal ? rect.width() : rect.height();
    int direction = (index > currentIndex()) ? 1 : -1;

    QRect oldEnd = rect;
    if (horizontal) {
        oldEnd.translate(-direction * offset, 0);
    } else {
        oldEnd.translate(0, -direction * offset);
    }

    QRect newStart = rect;
    if (horizontal) {
        newStart.translate(direction * offset, 0);
    } else {
        newStart.translate(0, direction * offset);
    }
    QRect newEnd = rect;

    newWidget->setGeometry(newStart);
    newWidget->show();
    newWidget->raise();

    QGraphicsOpacityEffect* oldFade = new QGraphicsOpacityEffect(oldWidget);
    oldWidget->setGraphicsEffect(oldFade);
    QGraphicsOpacityEffect* newFade = new QGraphicsOpacityEffect(newWidget);
    newWidget->setGraphicsEffect(newFade);

    QPropertyAnimation* oldAnimGeom
        = new QPropertyAnimation(oldWidget, "geometry");
    oldAnimGeom->setDuration(animationDuration_);
    oldAnimGeom->setStartValue(rect);
    oldAnimGeom->setEndValue(oldEnd);

    QPropertyAnimation* newAnimGeom
        = new QPropertyAnimation(newWidget, "geometry");
    newAnimGeom->setDuration(animationDuration_);
    newAnimGeom->setStartValue(newStart);
    newAnimGeom->setEndValue(newEnd);

    QPropertyAnimation* oldAnimFade
        = new QPropertyAnimation(oldFade, "opacity");
    oldAnimFade->setDuration(animationDuration_);
    oldAnimFade->setStartValue(1.0);
    oldAnimFade->setEndValue(0.0);

    QPropertyAnimation* newAnimFade
        = new QPropertyAnimation(newFade, "opacity");
    newAnimFade->setDuration(animationDuration_);
    newAnimFade->setStartValue(0.0);
    newAnimFade->setEndValue(1.0);

    QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
    group->addAnimation(oldAnimGeom);
    group->addAnimation(newAnimGeom);
    group->addAnimation(oldAnimFade);
    group->addAnimation(newAnimFade);

    connect(group, &QParallelAnimationGroup::finished, this, [=, this]() {
        setCurrentIndex(index);
        oldWidget->setGeometry(rect);
        newWidget->setGeometry(rect);
        oldWidget->setGraphicsEffect(nullptr);
        newWidget->setGraphicsEffect(nullptr);
        oldFade->deleteLater();
        newFade->deleteLater();
        group->deleteLater();
    });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

}
