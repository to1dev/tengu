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

#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

#include "izanagi/secp256k1/secp256k1_musig.h"

#ifdef __cplusplus
}
#endif

#include <QNetworkAccessManager>

#include <qcoro5/qcoro/QCoro>

#include "PSBT.h"

namespace bitcoin {

class INetworkManager {
public:
    virtual ~INetworkManager() = default;

    // Placeholder: Send MuSig2 nonce to peer
    virtual QCoro::Task<bool> sendNonce(
        const std::string& peer, const secp256k1_musig_pubnonce& nonce)
        = 0;

    // Placeholder: Receive MuSig2 nonce from peer
    virtual QCoro::Task<secp256k1_musig_pubnonce> receiveNonce(
        const std::string& peer)
        = 0;

    // Placeholder: Send PSBT to peer
    virtual QCoro::Task<bool> sendPSBT(
        const std::string& peer, const PSBT& psbt)
        = 0;

    // Placeholder: Receive PSBT from peer
    virtual QCoro::Task<PSBT> receivePSBT(
        const std::string& peer, secp256k1_context* ctx)
        = 0;
};
}
