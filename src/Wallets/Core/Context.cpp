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

#include "Context.h"

#include <sodium.h>

secp256k1_context* getSecpContext()
{
    static secp256k1_context* ctx = nullptr;
    static bool randomized = false;

    if (!ctx) {
        ctx = secp256k1_context_create(SECP256K1_CONTEXT_NONE);
        if (!ctx) {
            throw std::runtime_error("Failed to create secp256k1 context");
        }
    }

    if (!randomized) {
        unsigned char seed[32];
        randombytes_buf(seed, sizeof(seed));

        int ok = secp256k1_context_randomize(ctx, seed);
        if (!ok) {
            throw std::runtime_error("Failed to randomize secp256k1 context");
        }

        randomized = true;
    }

    return ctx;
}
