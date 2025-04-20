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

#ifndef GLOBALMANAGER_H
#define GLOBALMANAGER_H

#include "Managers/ResourceManager.h"
#include "Managers/SettingManager.h"
#include "Managers/ThemeManager.h"
#include "Managers/WindowManager.h"

#include "Clients/Core/Hydra/Hydra.h"
#include "Clients/Core/Hydra/PriceDataSource.h"

using namespace Daitengu::Clients::Hydra;

namespace Daitengu::Core {

class GlobalManager {
public:
    GlobalManager();

    ResourceManager* resourceManager() const;
    SettingManager* settingManager() const;
    ThemeManager* themeManager() const;
    WindowManager* windowManager() const;

    Hydra* hydra() const;

private:
    QApplication* app_;
    std::unique_ptr<ResourceManager> resourceManager_;
    std::unique_ptr<SettingManager> settingManager_;
    std::unique_ptr<ThemeManager> themeManager_;
    std::unique_ptr<WindowManager> windowManager_;

    std::unique_ptr<Hydra> hydra_;
};

}
#endif // GLOBALMANAGER_H
