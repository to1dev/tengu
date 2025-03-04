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

#include "PredefinedLayoutEngine.h"

namespace Daitengu::Layouts {

std::unique_ptr<LayoutEngine> PredefinedLayoutEngine::create(
    PredefinedType type)
{
    switch (type) {
    case PredefinedType::IDE_STYLE:
        return std::make_unique<GridLayoutEngine>(12, 12, 3, 3);

    case PredefinedType::DOCUMENT_COMPARE:
        return std::make_unique<GridLayoutEngine>(1, 2, 5, 5);

    case PredefinedType::TRIPLE_COLUMNS:
        return std::make_unique<GridLayoutEngine>(1, 3, 5, 5);

    case PredefinedType::QUAD_GRID:
        return std::make_unique<GridLayoutEngine>(2, 2, 5, 5);

    case PredefinedType::MASTER_DETAIL:
        return std::make_unique<GridLayoutEngine>(1, 2, 10, 0);

    case PredefinedType::PRESENTATION:
        return std::make_unique<GridLayoutEngine>(2, 1, 5, 5);

    default:
        return std::make_unique<GridLayoutEngine>(12, 12, 3, 3);
    }
}

}
