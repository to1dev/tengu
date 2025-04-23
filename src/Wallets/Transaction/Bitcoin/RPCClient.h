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

#include <string>
#include <vector>

#include "Transaction.h"

namespace bitcoin {

struct UTXO {
    std::array<unsigned char, 32> txid;
    uint32_t vout;
    uint64_t value;
    std::vector<unsigned char> script_pubkey;
};

class IRPCClient {
public:
    virtual ~IRPCClient() = default;

    // Placeholder: Get UTXOs for address
    virtual std::vector<UTXO> getUTXOs(const std::string& address) = 0;

    // Placeholder: Get current network fee rate (satoshi per byte)
    virtual uint64_t getFeeRate() = 0;

    // Placeholder: Broadcast transaction
    virtual std::string broadcastTransaction(const Transaction& tx) = 0;
};
}
