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

#include "AnimationController.h"

namespace Daitengu::Layouts {

AnimationController::AnimationController(QObject* parent)
    : QObject(parent)
    , group_(nullptr)
{
}

AnimationController::~AnimationController()
{
    if (group_) {
        group_->stop();
        delete group_;
    }
}

void AnimationController::animateTo(
    const QMap<QWidget*, QRect>& targetGeometries, int duration)
{
    if (group_) {
        group_->stop();
        delete group_;
        group_ = nullptr;
    }

    group_ = new QParallelAnimationGroup(this);

    for (auto it = targetGeometries.begin(); it != targetGeometries.end();
        ++it) {
        QWidget* w = it.key();
        if (!w)
            continue;
        QRect target = it.value();

        QPropertyAnimation* anim
            = new QPropertyAnimation(w, "geometry", group_);
        anim->setDuration(duration);
        anim->setStartValue(w->geometry());
        anim->setEndValue(target);
        // or easingCurve
        // anim->setEasingCurve(QEasingCurve::OutCubic);

        group_->addAnimation(anim);
    }

    connect(group_, &QParallelAnimationGroup::finished, this,
        &AnimationController::onGroupFinished);

    group_->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationController::onGroupFinished()
{
    Q_EMIT animationFinished();
    group_ = nullptr;
}

}
