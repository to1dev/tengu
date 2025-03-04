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

#ifndef LAYOUTORCHESTRATOR_H
#define LAYOUTORCHESTRATOR_H

#include <memory>

#include <QDebug>
#include <QGuiApplication>
#include <QMap>
#include <QObject>
#include <QRect>
#include <QScreen>

#include "AnimationController.h"
#include "LayoutEngine.h"
#include "ScreenManager.h"
#include "WindowState.h"

namespace Daitengu::Layouts {

class LayoutOrchestrator : public QObject {
    Q_OBJECT

public:
    explicit LayoutOrchestrator(QObject* parent = nullptr);
    ~LayoutOrchestrator() override;

    void setLayoutEngine(std::unique_ptr<LayoutEngine> newLayoutEngine);

    void registerWindow(QWidget* w);
    void unregisterWindow(QWidget* w);

    void snapToLayout();

    void animateToLayout(int duration = 300);

    ScreenManager* screenManager() const;

Q_SIGNALS:
    void layoutChanged();

private Q_SLOTS:
    void onScreensChanged();

private:
    std::unique_ptr<LayoutEngine> layoutEngine_;
    QList<WindowState*> windows_;
    AnimationController* animController_;
    ScreenManager* screenManager_;

    void applyLayout(const QMap<QWidget*, QRect>& layout);
};

}
#endif // LAYOUTORCHESTRATOR_H
