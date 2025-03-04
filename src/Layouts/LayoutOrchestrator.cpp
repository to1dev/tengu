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

#include "LayoutOrchestrator.h"

namespace Daitengu::Layouts {

LayoutOrchestrator::LayoutOrchestrator(QObject* parent)
    : QObject(parent)
    , layoutEngine_(nullptr)
    , animController_(new AnimationController(this))
    , screenManager_(new ScreenManager(this))
{
    connect(screenManager_, &ScreenManager::screensChanged, this,
        &LayoutOrchestrator::onScreensChanged);
}

LayoutOrchestrator::~LayoutOrchestrator()
{
    qDeleteAll(windows_);
    windows_.clear();
}

void LayoutOrchestrator::setLayoutEngine(
    std::unique_ptr<LayoutEngine> newLayoutEngine)
{
    layoutEngine_ = std::move(newLayoutEngine);
}

void LayoutOrchestrator::registerWindow(QWidget* w)
{
    if (!w)
        return;
    for (auto* ws : windows_) {
        if (ws->widget() == w) {
            return;
        }
    }
    auto* state = new WindowState(w);
    windows_.append(state);
}

void LayoutOrchestrator::unregisterWindow(QWidget* w)
{
    for (int i = 0; i < windows_.size(); ++i) {
        if (windows_[i]->widget() == w) {
            delete windows_[i];
            windows_.removeAt(i);
            break;
        }
    }
}

void LayoutOrchestrator::snapToLayout()
{
    if (!layoutEngine_) {
        qWarning() << "[LayoutOrchestrator] No LayoutEngine set!";
        return;
    }
    if (auto* primary = QGuiApplication::primaryScreen()) {
        QRect screenGeom = primary->availableGeometry();
        auto layout = layoutEngine_->calculateLayout(windows_, screenGeom);
        applyLayout(layout);
    }
}

void LayoutOrchestrator::animateToLayout(int duration)
{
    if (!layoutEngine_) {
        qWarning() << "[LayoutOrchestrator] No LayoutEngine set!";
        return;
    }
    if (auto* primary = QGuiApplication::primaryScreen()) {
        QRect screenGeom = primary->availableGeometry();
        auto layout = layoutEngine_->calculateLayout(windows_, screenGeom);
        animController_->animateTo(layout, duration);
    }
}

ScreenManager* LayoutOrchestrator::screenManager() const
{
    return screenManager_;
}

void LayoutOrchestrator::onScreensChanged()
{
    snapToLayout();
}

void LayoutOrchestrator::applyLayout(const QMap<QWidget*, QRect>& layout)
{
    for (auto it = layout.begin(); it != layout.end(); ++it) {
        if (QWidget* w = it.key()) {
            w->setGeometry(it.value());
        }
    }
    Q_EMIT layoutChanged();
}

}
