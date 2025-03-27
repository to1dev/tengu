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

#include "MempoolSpaceBitcoinProvider.h"

namespace Daitengu::Wallets {

MempoolSpaceBitcoinProvider::MempoolSpaceBitcoinProvider(QObject* parent)
    : BlockchainProvider(parent)
{
}

MempoolSpaceBitcoinProvider::~MempoolSpaceBitcoinProvider()
{
}

QFuture<ProviderResponse<BalanceResult>>
MempoolSpaceBitcoinProvider::getBalance(const QString& address)
{
    return QFuture<ProviderResponse<BalanceResult>>();
}

QFuture<ProviderResponse<TokenList>> MempoolSpaceBitcoinProvider::getTokens(
    const QString& address)
{
    return QFuture<ProviderResponse<TokenList>>();
}

QFuture<ProviderResponse<bool>> MempoolSpaceBitcoinProvider::isValidAddress(
    const QString& address)
{
    return QFuture<ProviderResponse<bool>>();
}

bool MempoolSpaceBitcoinProvider::subscribeToAddressChanges(
    const QString& address)
{
    return false;
}

void MempoolSpaceBitcoinProvider::unsubscribeFromAddressChanges(
    const QString& address)
{
}

bool MempoolSpaceBitcoinProvider::initialize()
{
    return false;
}

void MempoolSpaceBitcoinProvider::shutdown()
{
}

void MempoolSpaceBitcoinProvider::onWebSocketConnected()
{
}

void MempoolSpaceBitcoinProvider::onWebSocketDisconnected()
{
}

void MempoolSpaceBitcoinProvider::onWebSocketTextMessageReceived(
    const QString& message)
{
}

void MempoolSpaceBitcoinProvider::onWebSocketError(
    QAbstractSocket::SocketError error)
{
}

void MempoolSpaceBitcoinProvider::onSslErrors(const QList<QSslError>& errors)
{
}

void MempoolSpaceBitcoinProvider::setupPingPong()
{
}

void MempoolSpaceBitcoinProvider::checkPingPongHealth()
{
}

ProviderResponse<BalanceResult>
MempoolSpaceBitcoinProvider::parseBalanceResponse(
    const nlohmann::json& response)
{
    return ProviderResponse<BalanceResult>();
}
}
