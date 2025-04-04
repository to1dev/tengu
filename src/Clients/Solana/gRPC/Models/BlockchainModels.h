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

#include <cstdint>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Wallets/Core/Types.h"
using namespace Daitengu::Wallets;

namespace Daitengu::Clients::Solana::gRPC::Models {

struct TokenBalance {
    std::string owner;
    std::string mint;
    double amount;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TokenBalance, owner, mint, amount)
};

struct SwapTransaction {
    std::string signature;
    std::string walletAddress;
    std::string dexType;
    std::string tokenMint;
    std::string tokenSymbol;
    double tokenAmount;
    double solAmount;
    bool isBuy;
    uint64_t blockTime;
    double usdValue;
    std::vector<TokenBalance> preBalances;
    std::vector<TokenBalance> postBalances;

    std::string getKey() const
    {
        return "tx:" + signature;
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SwapTransaction, signature, walletAddress,
        dexType, tokenMint, tokenSymbol, tokenAmount, solAmount, isBuy,
        blockTime, usdValue, preBalances, postBalances)
};

struct EthereumSwap {
    std::string txHash;
    std::string dexName;
    std::string tokenAddress;
    std::string walletAddress;
    uint64_t blockNumber;
    uint64_t timestamp;
    double tokenAmount;
    double ethAmount;
    bool isBuy;
    double usdValue;

    std::string getKey() const
    {
        return "eth:tx:" + txHash;
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(EthereumSwap, txHash, dexName, tokenAddress,
        walletAddress, blockNumber, timestamp, tokenAmount, ethAmount, isBuy,
        usdValue)
};

struct WalletScore {
    std::string address;
    double profitability;
    int successfulTrades;
    int totalTrades;
    double avgTransactionSize;
    uint64_t firstSeenTime;
    uint64_t lastUpdateTime;

    double getSuccessRate() const
    {
        return totalTrades > 0
            ? static_cast<double>(successfulTrades) / totalTrades
            : 0.0;
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(WalletScore, address, profitability,
        successfulTrades, totalTrades, avgTransactionSize, firstSeenTime,
        lastUpdateTime)
};

struct BlockchainEvent {
    ChainType chain;
    uint64_t timestamp;

    virtual ~BlockchainEvent() = default;
    virtual json toJson() const = 0;
};

struct SolanaEvent : public BlockchainEvent {
    enum class Type {
        TRANSACTION,
        ACCOUNT,
        SLOT,
    };

    Type type;
    std::string data;

    SolanaEvent()
    {
        chain = ChainType::SOLANA;
    }

    json toJson() const override
    {
        json j;
        j["chain"] = "solana";
        j["timestamp"] = timestamp;
        j["type"] = static_cast<int>(type);
        j["data"] = data;
        return j;
    }
};

struct EthereumEvent : public BlockchainEvent {
    enum class Type {
        BLOCK,
        TRANSACTION,
        LOG,
    };

    Type type;
    std::string data;

    EthereumEvent()
    {
        chain = ChainType::ETHEREUM;
    }

    json toJson() const override
    {
        json j;
        j["chain"] = "ethereum";
        j["timestamp"] = timestamp;
        j["type"] = static_cast<int>(type);
        j["data"] = data;
        return j;
    }
};
}
