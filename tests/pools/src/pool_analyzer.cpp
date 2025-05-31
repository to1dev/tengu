#include "pool_analyzer.h"
#include "config.h"
#include "math_utils.h"
#include "rpc_client.h"

#include <algorithm>
#include <sstream>

PoolAnalyzer::PoolAnalyzer(const TokenManager& tm)
    : tokenManager(tm)
{
}

std::string PoolAnalyzer::getPoolAddress(
    const std::string& tokenA, const std::string& tokenB, uint32_t fee)
{
    std::stringstream feeHex;
    feeHex << std::hex << std::setfill('0') << std::setw(8) << fee;

    std::string data = Config::GET_POOL_FUNC + std::string(24, '0')
        + tokenA.substr(2) + std::string(24, '0') + tokenB.substr(2)
        + std::string(56, '0') + feeHex.str();

    json result = RpcClient::callContract(Config::UNISWAP_V3_FACTORY, data);
    std::string poolAddress = result.get<std::string>();

    if (poolAddress.length() >= 42) {
        std::string realAddress
            = "0x" + poolAddress.substr(poolAddress.length() - 40);
        if (realAddress != "0x0000000000000000000000000000000000000000") {
            return MathUtils::toLowerCase(realAddress);
        }
    }
    return "";
}

uint64_t PoolAnalyzer::getPoolLiquidity(const std::string& poolAddress)
{
    try {
        json result
            = RpcClient::callContract(poolAddress, Config::LIQUIDITY_FUNC);
        std::string liquidityHex = result.get<std::string>();
        if (liquidityHex.length() > 2) {
            return std::stoull(liquidityHex.substr(2), nullptr, 16);
        }
    } catch (...) {
        // 忽略错误
    }
    return 0;
}

uint32_t PoolAnalyzer::getPoolActualFee(const std::string& poolAddress)
{
    try {
        json result = RpcClient::callContract(poolAddress, Config::FEE_FUNC);
        std::string feeHex = result.get<std::string>();
        if (feeHex.length() > 2) {
            return std::stoul(feeHex.substr(2), nullptr, 16);
        }
    } catch (...) {
        // 忽略错误
    }
    return 0;
}

std::pair<std::string, std::string> PoolAnalyzer::getPoolTokens(
    const std::string& poolAddress)
{
    try {
        json token0Result
            = RpcClient::callContract(poolAddress, Config::TOKEN0_FUNC);
        json token1Result
            = RpcClient::callContract(poolAddress, Config::TOKEN1_FUNC);

        std::string token0Hex = token0Result.get<std::string>();
        std::string token1Hex = token1Result.get<std::string>();

        std::string token0 = MathUtils::toLowerCase(
            "0x" + token0Hex.substr(token0Hex.length() - 40));
        std::string token1 = MathUtils::toLowerCase(
            "0x" + token1Hex.substr(token1Hex.length() - 40));

        return { token0, token1 };
    } catch (const std::exception& e) {
        return { "", "" };
    }
}

std::string PoolAnalyzer::extractSqrtPriceX96(const std::string& slot0Hex)
{
    if (slot0Hex.length() < 66)
        return "";

    std::string data = slot0Hex.substr(2);
    std::string sqrtPriceField = data.substr(0, 64);
    std::string sqrtPriceX96 = sqrtPriceField.substr(24, 40);

    size_t firstNonZero = sqrtPriceX96.find_first_not_of('0');
    if (firstNonZero != std::string::npos) {
        return sqrtPriceX96.substr(firstNonZero);
    }

    return "0";
}

PoolInfo PoolAnalyzer::getBestPool(
    const std::string& tokenA, const std::string& tokenB)
{
    PoolInfo bestPool;
    uint64_t maxLiquidity = 0;

    std::vector<std::pair<std::string, std::string>> tokenPairs
        = { { tokenA, tokenB }, { tokenB, tokenA } };

    for (const auto& [tA, tB] : tokenPairs) {
        for (uint32_t searchFee : Config::STANDARD_FEES) {
            try {
                std::string poolAddress = getPoolAddress(tA, tB, searchFee);

                if (!poolAddress.empty()) {
                    uint64_t liquidity = getPoolLiquidity(poolAddress);

                    if (liquidity < Config::MIN_LIQUIDITY) {
                        continue;
                    }

                    auto [token0, token1] = getPoolTokens(poolAddress);

                    json result = RpcClient::callContract(
                        poolAddress, Config::SLOT0_FUNC);
                    std::string slot0Hex = result.get<std::string>();
                    std::string sqrtPriceX96Hex = extractSqrtPriceX96(slot0Hex);

                    if (!sqrtPriceX96Hex.empty() && sqrtPriceX96Hex != "0") {
                        uint8_t decimals0 = tokenManager.getDecimals(token0);
                        uint8_t decimals1 = tokenManager.getDecimals(token1);

                        long double price = MathUtils::sqrtPriceX96ToPrice(
                            sqrtPriceX96Hex, decimals0, decimals1);

                        uint32_t actualFee = getPoolActualFee(poolAddress);

                        if (liquidity > maxLiquidity && price > 0.0L) {
                            bestPool.address = poolAddress;
                            bestPool.searchFee = searchFee;
                            bestPool.actualFee = actualFee;
                            bestPool.liquidity = liquidity;
                            bestPool.price = price;
                            bestPool.token0 = token0;
                            bestPool.token1 = token1;
                            bestPool.valid = true;
                            bestPool.feeMatched = (searchFee == actualFee);
                            maxLiquidity = liquidity;
                        }
                    }
                }
            } catch (...) {
                continue;
            }
        }
    }

    return bestPool;
}

bool PoolAnalyzer::hasValidPool(
    const std::string& tokenA, const std::string& tokenB)
{
    for (uint32_t fee : Config::STANDARD_FEES) {
        try {
            std::string poolAddress = getPoolAddress(tokenA, tokenB, fee);
            if (!poolAddress.empty()) {
                uint64_t liquidity = getPoolLiquidity(poolAddress);
                if (liquidity >= Config::MIN_LIQUIDITY) {
                    return true;
                }
            }
        } catch (...) {
            continue;
        }
    }
    return false;
}
