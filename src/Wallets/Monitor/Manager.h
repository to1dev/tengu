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

#pragma once

#include <memory>
#include <unordered_map>

#include "Handler.h"
#include "Wallets/Core/Types.h"

namespace Daitengu::Wallets {

class Manager {
public:
    explicit Manager();
    void registerHandler(ChainType chain, std::unique_ptr<Handler> handler);
    std::optional<Handler*> getHandler(ChainType chain);
    void setPreferredApiSource(ChainType chain, const QString& apiEndpoint);

private:
    std::unordered_map<ChainType, std::unique_ptr<Handler>> handlers_;
};
}
