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

#include "EthereumHandler.h"

#include "../Providers/EthereumBlockscoutProvider.h"
#include "../Providers/EthereumPublicNodeProvider.h"

#include <QNetworkReply>

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Daitengu::Wallets {

EthereumHandler::EthereumHandler()
{
    providers_.push_back(std::make_unique<EthereumPublicNodeProvider>());
    providers_.push_back(std::make_unique<EthereumBlockscoutProvider>());
    currentProvider_ = providers_[0].get();
}

QCoro::Task<std::optional<Monitor::BalanceResult>>
EthereumHandler::fetchBalance(
    QNetworkAccessManager* networkManager, const QString& address)
{
    co_return co_await currentProvider_->fetchBalance(
        networkManager, address, ChainType::ETHEREUM);
}

void EthereumHandler::setPreferredApiSource(const QString& apiEndpoint)
{
    for (auto& provider : providers_) {
        if (provider->getApiEndpoint() == apiEndpoint) {
            currentProvider_ = provider.get();
            spdlog::info(
                "Ethereum API source set to {}", apiEndpoint.toStdString());
            return;
        }
    }
    spdlog::warn("Ethereum API source {} not found, keeping current provider",
        apiEndpoint.toStdString());
}

QString EthereumHandler::getCurrentApiSource() const
{
    return currentProvider_->getApiEndpoint();
}
}
