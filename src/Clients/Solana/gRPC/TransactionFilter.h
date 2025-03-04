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

#ifndef TRANSACTIONFILTER_H
#define TRANSACTIONFILTER_H

#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "geyser/geyser.grpc.pb.h"

namespace Daitengu::Clients::Solana {

class TransactionFilter {
public:
    virtual ~TransactionFilter() = default;

    virtual void processTransaction(
        const geyser::SubscribeUpdateTransaction& tx)
        = 0;
};

}
#endif // TRANSACTIONFILTER_H
