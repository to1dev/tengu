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

#ifndef ANIMATIONCONTROLLER_H
#define ANIMATIONCONTROLLER_H

#include <QMap>
#include <QObject>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QRect>
#include <QWidget>

namespace Daitengu::Layouts {

class AnimationController : public QObject {
    Q_OBJECT

public:
    explicit AnimationController(QObject* parent = nullptr);
    ~AnimationController() override;

    void animateTo(
        const QMap<QWidget*, QRect>& targetGeometries, int duration = 300);

Q_SIGNALS:
    void animationFinished();

private Q_SLOTS:
    void onGroupFinished();

private:
    QParallelAnimationGroup* group_;
};

}
#endif // ANIMATIONCONTROLLER_H
