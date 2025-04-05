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

#include <unordered_set>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include <spdlog/spdlog.h>

#include "TransactionFilter.h"

namespace Daitengu::Clients::Solana::gRPC {

class DexFilter : public TransactionFilter {
public:
    explicit DexFilter(std::unordered_set<std::string> dexPrograms);
    void processTransaction(
        const geyser::SubscribeUpdateTransaction& tx) override;
    void updateConfig(const std::string& config) override;

    std::string name() const override
    {
        return "DexFilter";
    }

private:
    std::unordered_set<std::string> dexPrograms_;
};

}
