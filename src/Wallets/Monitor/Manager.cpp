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

#include "Manager.h"
#include "Factory.h"

#include <spdlog/spdlog.h>

namespace Daitengu::Wallets {

Manager::Manager()
{
    registerHandler(
        ChainType::BITCOIN, Factory::createHandler(ChainType::BITCOIN));
    registerHandler(
        ChainType::ETHEREUM, Factory::createHandler(ChainType::ETHEREUM));
    registerHandler(
        ChainType::SOLANA, Factory::createHandler(ChainType::SOLANA));
}

void Manager::registerHandler(ChainType chain, std::unique_ptr<Handler> handler)
{
    handlers_[chain] = std::move(handler);
    spdlog::info("Registered handler for chain {}", static_cast<int>(chain));
}

std::optional<Handler*> Manager::getHandler(ChainType chain)
{
    if (handlers_.find(chain) != handlers_.end()) {
        auto ptr = handlers_.at(chain).get();
        if (ptr != nullptr) {
            return ptr;
        }
    }

    return std::nullopt;
}

void Manager::setPreferredApiSource(ChainType chain, const QString& apiEndpoint)
{
    auto handlerOpt = getHandler(chain);
    if (handlerOpt) {
        handlerOpt.value()->setPreferredApiSource(apiEndpoint);
    }
}
}
