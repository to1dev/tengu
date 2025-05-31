#include "report_generator.h"
#include "config.h"
#include "math_utils.h"
#include "rpc_client.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

ReportGenerator::ReportGenerator(const TokenManager& tm, PoolAnalyzer& pa)
    : tokenManager(tm)
    , poolAnalyzer(pa)
{
}

void ReportGenerator::printPoolDetails(const std::string& tokenA,
    const std::string& tokenB, const std::string& nameA,
    const std::string& nameB)
{
    std::cout << "\n=== " << nameA << "/" << nameB << " Pools ===" << std::endl;

    bool foundAnyPool = false;

    for (uint32_t searchFee : Config::STANDARD_FEES) {
        try {
            std::string poolAddress
                = poolAnalyzer.getPoolAddress(tokenA, tokenB, searchFee);

            if (!poolAddress.empty()) {
                uint64_t liquidity = poolAnalyzer.getPoolLiquidity(poolAddress);

                if (liquidity < Config::MIN_LIQUIDITY) {
                    continue;
                }

                foundAnyPool = true;
                std::cout << "\nPool Address: " << poolAddress << std::endl;

                // 获取并验证实际费率
                uint32_t actualFee = poolAnalyzer.getPoolActualFee(poolAddress);
                std::cout << "Search Fee: " << searchFee / 10000.0 << "% ("
                          << searchFee << " bps)" << std::endl;
                std::cout << "Actual Fee: " << actualFee / 10000.0 << "% ("
                          << actualFee << " bps)";

                if (searchFee != actualFee) {
                    std::cout << " [MISMATCH WARNING!]";
                }
                std::cout << std::endl;

                std::cout << "Liquidity: " << liquidity << std::endl;

                auto [token0, token1] = poolAnalyzer.getPoolTokens(poolAddress);
                std::string symbol0 = tokenManager.getSymbol(token0);
                std::string symbol1 = tokenManager.getSymbol(token1);

                std::cout << "Pool tokens: " << symbol0 << " (token0) / "
                          << symbol1 << " (token1)" << std::endl;

                json result
                    = RpcClient::callContract(poolAddress, Config::SLOT0_FUNC);
                std::string slot0Hex = result.get<std::string>();
                std::string sqrtPriceX96Hex
                    = poolAnalyzer.extractSqrtPriceX96(slot0Hex);

                if (!sqrtPriceX96Hex.empty() && sqrtPriceX96Hex != "0") {
                    uint8_t decimals0 = tokenManager.getDecimals(token0);
                    uint8_t decimals1 = tokenManager.getDecimals(token1);

                    long double price = MathUtils::sqrtPriceX96ToPrice(
                        sqrtPriceX96Hex, decimals0, decimals1);

                    std::cout << "Price: 1 " << symbol0 << " = "
                              << MathUtils::formatHighPrecision(price) << " "
                              << symbol1 << std::endl;
                    std::cout << "Price: 1 " << symbol1 << " = "
                              << MathUtils::formatHighPrecision(1.0L / price)
                              << " " << symbol0 << std::endl;
                }

                std::cout << "----------------------------------------"
                          << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Error checking fee " << searchFee / 10000.0
                      << "%: " << e.what() << std::endl;
        }
    }

    if (!foundAnyPool) {
        std::cout
            << "No pools with sufficient liquidity found for this token pair"
            << std::endl;
    }
}

void ReportGenerator::printArbitrageOpportunity(const ArbitrageOpportunity& opp)
{
    std::cout << "\n============================================" << std::endl;
    std::cout << "Triangle Arbitrage: " << opp.nameA << " -> " << opp.nameB
              << " -> " << opp.nameC << " -> " << opp.nameA << std::endl;
    std::cout << "============================================" << std::endl;

    std::cout << "Triangle pools found:" << std::endl;
    std::cout << opp.nameA << " <-> " << opp.nameB << ": " << opp.poolAB.address
              << " (Actual Fee: " << opp.poolAB.actualFee / 10000.0
              << "%, Liq: " << opp.poolAB.liquidity << ")" << std::endl;
    std::cout << opp.nameB << " <-> " << opp.nameC << ": " << opp.poolBC.address
              << " (Actual Fee: " << opp.poolBC.actualFee / 10000.0
              << "%, Liq: " << opp.poolBC.liquidity << ")" << std::endl;
    std::cout << opp.nameC << " <-> " << opp.nameA << ": " << opp.poolCA.address
              << " (Actual Fee: " << opp.poolCA.actualFee / 10000.0
              << "%, Liq: " << opp.poolCA.liquidity << ")" << std::endl;

    // 显示费率匹配状态
    if (!opp.poolAB.feeMatched || !opp.poolBC.feeMatched
        || !opp.poolCA.feeMatched) {
        std::cout << "WARNING: Some pools have fee mismatches!" << std::endl;
    }

    std::cout << "\nHigh-precision exchange rates:" << std::endl;
    std::cout << "1 " << opp.nameA << " -> "
              << MathUtils::formatHighPrecision(opp.rateAB) << " " << opp.nameB
              << std::endl;
    std::cout << "1 " << opp.nameB << " -> "
              << MathUtils::formatHighPrecision(opp.rateBC) << " " << opp.nameC
              << std::endl;
    std::cout << "1 " << opp.nameC << " -> "
              << MathUtils::formatHighPrecision(opp.rateCA) << " " << opp.nameA
              << std::endl;

    std::cout << "\nArbitrage Analysis (High Precision):" << std::endl;
    std::cout << "Expected return (before fees): "
              << MathUtils::formatHighPrecision(opp.expectedReturn)
              << std::endl;
    std::cout << "Net return (after fees): "
              << MathUtils::formatHighPrecision(opp.netReturn) << std::endl;
    std::cout << "Profit percentage: "
              << MathUtils::formatHighPrecision(opp.profitPercent) << "%"
              << std::endl;

    if (std::abs(opp.profitPercent) > Config::MIN_ARBITRAGE_PROFIT) {
        if (opp.profitPercent > 0) {
            std::cout << "\n*** ARBITRAGE OPPORTUNITY DETECTED! ***"
                      << std::endl;
            std::cout << "Direction: " << opp.nameA << " -> " << opp.nameB
                      << " -> " << opp.nameC << " -> " << opp.nameA
                      << std::endl;
        } else {
            std::cout << "\n*** REVERSE ARBITRAGE OPPORTUNITY ***" << std::endl;
            std::cout << "Direction: " << opp.nameA << " -> " << opp.nameC
                      << " -> " << opp.nameB << " -> " << opp.nameA
                      << std::endl;
        }
        std::cout << "Estimated profit: "
                  << MathUtils::formatHighPrecision(std::abs(opp.profitPercent))
                  << "%" << std::endl;
    } else {
        std::cout << "\nNo significant arbitrage opportunity found."
                  << std::endl;
    }

    std::cout << "Minimum liquidity in path: " << opp.minLiquidity << std::endl;

    // 风险评估
    printRiskAssessment(opp);
}

void ReportGenerator::printRiskAssessment(const ArbitrageOpportunity& opp)
{
    std::cout << "\nRisk Assessment:" << std::endl;

    // 流动性风险
    if (opp.minLiquidity < 10000000) { // < 10M
        std::cout << "WARNING: Low liquidity may cause high slippage!"
                  << std::endl;
    } else {
        std::cout << "Liquidity: Acceptable for small to medium trades"
                  << std::endl;
    }

    // 利润风险
    if (std::abs(opp.profitPercent) < 0.01L) {
        std::cout
            << "WARNING: Very small profit margin, may not cover gas costs!"
            << std::endl;
    }

    // 稳定币风险评估
    bool hasStablecoin = tokenManager.isStablecoin(opp.tokenA)
        || tokenManager.isStablecoin(opp.tokenB)
        || tokenManager.isStablecoin(opp.tokenC);

    if (hasStablecoin) {
        std::cout << "Risk Level: LOW (involves stablecoins)" << std::endl;
    } else {
        std::cout << "Risk Level: HIGH (volatile assets only)" << std::endl;
    }
}

void ReportGenerator::printSummaryReport(
    const std::vector<ArbitrageOpportunity>& opportunities)
{
    std::cout << "\n=== ARBITRAGE SUMMARY REPORT ===" << std::endl;

    if (opportunities.empty()) {
        std::cout << "No profitable arbitrage opportunities found."
                  << std::endl;
        return;
    }

    std::cout << "Found " << opportunities.size()
              << " profitable opportunities:" << std::endl;

    // 按利润排序
    std::vector<ArbitrageOpportunity> sortedOps = opportunities;
    std::sort(sortedOps.begin(), sortedOps.end(),
        [](const ArbitrageOpportunity& a, const ArbitrageOpportunity& b) {
            return std::abs(a.profitPercent) > std::abs(b.profitPercent);
        });

    std::cout << "\nTop opportunities (by profit %):" << std::endl;
    for (size_t i = 0; i < std::min(size_t(5), sortedOps.size()); i++) {
        const auto& opp = sortedOps[i];
        std::cout << (i + 1) << ". " << opp.nameA << "-" << opp.nameB << "-"
                  << opp.nameC << ": "
                  << MathUtils::formatHighPrecision(std::abs(opp.profitPercent))
                  << "% (Liquidity: " << opp.minLiquidity << ")" << std::endl;
    }
}
