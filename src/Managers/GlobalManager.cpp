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

#include "GlobalManager.h"

namespace Daitengu::Core {

GlobalManager::GlobalManager()
    : app_(qApp)
{
    settingManager_ = std::make_unique<SettingManager>();
    themeManager_ = std::make_unique<ThemeManager>();
    resourceManager_ = std::make_unique<ResourceManager>();
    windowManager_ = std::make_unique<WindowManager>();

    hydra_ = std::make_unique<Hydra>();
    auto priceSource
        = std::make_unique<PriceDataSource>("CryptoPrices", 30, 5000);
    hydra_->addSource(std::move(priceSource));
    hydra_->startAll();
}

ResourceManager* GlobalManager::resourceManager() const
{
    return resourceManager_.get();
}

SettingManager* GlobalManager::settingManager() const
{
    return settingManager_.get();
}

ThemeManager* GlobalManager::themeManager() const
{
    return themeManager_.get();
}

WindowManager* GlobalManager::windowManager() const
{
    return windowManager_.get();
}

Hydra* GlobalManager::hydra() const
{
    return hydra_.get();
}
}
