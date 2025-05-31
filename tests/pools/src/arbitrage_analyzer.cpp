#include "arbitrage_analyzer.h"
#include "config.h"
#include "math_utils.h"

#include <algorithm>
#include <iostream>

ArbitrageAnalyzer::ArbitrageAnalyzer(const TokenManager& tm, PoolAnalyzer& pa)
    : tokenManager(tm)
    , poolAnalyzer(pa)
{
}

ArbitrageOpportunity ArbitrageAnalyzer::analyzeTriangleArbitrage(
    const std::string& tokenA, const std::string& tokenB,
    const std::string& tokenC)
{

    ArbitrageOpportunity opportunity;
    opportunity.tokenA = tokenA;
    opportunity.tokenB = tokenB;
    opportunity.tokenC = tokenC;
    opportunity.nameA = tokenManager.getSymbol(tokenA);
    opportunity.nameB = tokenManager.getSymbol(tokenB);
    opportunity.nameC = tokenManager.getSymbol(tokenC);

    // 获取三个池子
    opportunity.poolAB = poolAnalyzer.getBestPool(tokenA, tokenB);
    opportunity.poolBC = poolAnalyzer.getBestPool(tokenB, tokenC);
    opportunity.poolCA = poolAnalyzer.getBestPool(tokenC, tokenA);

    if (!opportunity.poolAB.valid || !opportunity.poolBC.valid
        || !opportunity.poolCA.valid) {
        return opportunity; // 返回无效机会
    }

    // 计算汇率
    calculateExchangeRates(opportunity);

    // 计算套利收益
    calculateArbitrageReturns(opportunity);

    // 检查是否符合套利条件
    opportunity.isValid
        = (std::abs(opportunity.profitPercent) > Config::MIN_ARBITRAGE_PROFIT);

    return opportunity;
}

void ArbitrageAnalyzer::calculateExchangeRates(ArbitrageOpportunity& opp)
{
    std::string tokenALower = MathUtils::toLowerCase(opp.tokenA);
    std::string tokenBLower = MathUtils::toLowerCase(opp.tokenB);
    std::string tokenCLower = MathUtils::toLowerCase(opp.tokenC);

    // A -> B
    if (tokenALower == opp.poolAB.token0) {
        opp.rateAB = opp.poolAB.price;
    } else {
        opp.rateAB = 1.0L / opp.poolAB.price;
    }

    // B -> C
    if (tokenBLower == opp.poolBC.token0) {
        opp.rateBC = opp.poolBC.price;
    } else {
        opp.rateBC = 1.0L / opp.poolBC.price;
    }

    // C -> A
    if (tokenCLower == opp.poolCA.token0) {
        opp.rateCA = opp.poolCA.price;
    } else {
        opp.rateCA = 1.0L / opp.poolCA.price;
    }
}

void ArbitrageAnalyzer::calculateArbitrageReturns(ArbitrageOpportunity& opp)
{
    opp.expectedReturn = opp.rateAB * opp.rateBC * opp.rateCA;

    // 使用实际费率计算
    long double feeMultiplier = (1.0L - opp.poolAB.actualFee / 1000000.0L)
        * (1.0L - opp.poolBC.actualFee / 1000000.0L)
        * (1.0L - opp.poolCA.actualFee / 1000000.0L);

    opp.netReturn = opp.expectedReturn * feeMultiplier;
    opp.profitPercent = (opp.netReturn - 1.0L) * 100.0L;

    opp.minLiquidity = std::min(
        { opp.poolAB.liquidity, opp.poolBC.liquidity, opp.poolCA.liquidity });
}

std::vector<ArbitrageOpportunity>
ArbitrageAnalyzer::scanForArbitrageOpportunities(
    const std::vector<std::tuple<std::string, std::string, std::string>>&
        triangles)
{

    std::vector<ArbitrageOpportunity> opportunities;

    std::cout << "Scanning " << triangles.size()
              << " triangle combinations for arbitrage..." << std::endl;

    int validTriangles = 0;
    int profitableTriangles = 0;

    for (const auto& [tokenA, tokenB, tokenC] : triangles) {
        ArbitrageOpportunity opp
            = analyzeTriangleArbitrage(tokenA, tokenB, tokenC);

        if (opp.poolAB.valid && opp.poolBC.valid && opp.poolCA.valid) {
            validTriangles++;

            if (opp.isValid) {
                profitableTriangles++;
                opportunities.push_back(opp);

                std::cout << "POTENTIAL OPPORTUNITY: " << opp.nameA << "-"
                          << opp.nameB << "-" << opp.nameC << " (Profit: "
                          << MathUtils::formatHighPrecision(
                                 std::abs(opp.profitPercent))
                          << "%)" << std::endl;
            }
        }
    }

    std::cout << "Scan Results:" << std::endl;
    std::cout << "- Valid triangles: " << validTriangles << "/"
              << triangles.size() << std::endl;
    std::cout << "- Profitable opportunities: " << profitableTriangles
              << std::endl;

    return opportunities;
}
