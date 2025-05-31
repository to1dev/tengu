#ifndef ARBITRAGE_ANALYZER_H
#define ARBITRAGE_ANALYZER_H

#include "pool_analyzer.h"
#include "token_manager.h"
#include "types.h"
#include <tuple>
#include <vector>

class ArbitrageAnalyzer {
private:
    const TokenManager& tokenManager;
    PoolAnalyzer& poolAnalyzer;

    void calculateExchangeRates(ArbitrageOpportunity& opp);
    void calculateArbitrageReturns(ArbitrageOpportunity& opp);

public:
    ArbitrageAnalyzer(const TokenManager& tm, PoolAnalyzer& pa);

    ArbitrageOpportunity analyzeTriangleArbitrage(const std::string& tokenA,
        const std::string& tokenB, const std::string& tokenC);

    std::vector<ArbitrageOpportunity> scanForArbitrageOpportunities(
        const std::vector<std::tuple<std::string, std::string, std::string>>&
            triangles);
};

#endif // ARBITRAGE_ANALYZER_H
