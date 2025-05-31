#ifndef POOL_ANALYZER_H
#define POOL_ANALYZER_H

#include "token_manager.h"
#include "types.h"

#include <string>

class PoolAnalyzer {
private:
    const TokenManager& tokenManager;

public:
    PoolAnalyzer(const TokenManager& tm);

    std::string getPoolAddress(
        const std::string& tokenA, const std::string& tokenB, uint32_t fee);
    uint64_t getPoolLiquidity(const std::string& poolAddress);
    uint32_t getPoolActualFee(const std::string& poolAddress);
    std::pair<std::string, std::string> getPoolTokens(
        const std::string& poolAddress);
    std::string extractSqrtPriceX96(const std::string& slot0Hex);

    PoolInfo getBestPool(const std::string& tokenA, const std::string& tokenB);
    bool hasValidPool(const std::string& tokenA, const std::string& tokenB);
};

#endif // POOL_ANALYZER_H
