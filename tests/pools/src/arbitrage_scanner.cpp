#include "arbitrage_scanner.h"
#include "config.h"

#include <iostream>

ArbitrageScanner::ArbitrageScanner()
    : poolAnalyzer(tokenManager)
    , arbitrageAnalyzer(tokenManager, poolAnalyzer)
    , reportGenerator(tokenManager, poolAnalyzer)
{
}

void ArbitrageScanner::run()
{
    printHeader();
    printConfiguration();
    analyzeMajorPools();
    scanAllPairs();
    performArbitrageAnalysis();
    printRecommendations();
}

void ArbitrageScanner::printHeader()
{
    std::cout << "Advanced Uniswap V3 Pool & Arbitrage Scanner" << std::endl;
    std::cout << "===========================================" << std::endl;
}

void ArbitrageScanner::printConfiguration()
{
    std::cout << "Configuration:" << std::endl;
    std::cout << "- Minimum liquidity requirement: " << Config::MIN_LIQUIDITY
              << std::endl;
    std::cout << "- Minimum arbitrage profit: " << Config::MIN_ARBITRAGE_PROFIT
              << "%" << std::endl;
    std::cout << "- Total tokens configured: " << tokenManager.getTokenCount()
              << std::endl;
    std::cout << "- Scanning Arbitrum network" << std::endl;

    auto tradingPairs = tokenManager.generateTradingPairs();
    std::cout << "- Generated " << tradingPairs.size() << " trading pairs"
              << std::endl;
}

void ArbitrageScanner::analyzeMajorPools()
{
    std::cout << "\n=== MAJOR POOLS ANALYSIS ===" << std::endl;

    reportGenerator.printPoolDetails(
        "0x82af49447d8a07e3bd95bd0d56f35241523fbab1",
        "0xff970a61a04b1ca14834a43f5de4533ebddb5cc8", "WETH", "USDC");

    reportGenerator.printPoolDetails(
        "0x82af49447d8a07e3bd95bd0d56f35241523fbab1",
        "0xfd086bc7cd5c481dcc9c85ebe478a1c0b69fcbb9", "WETH", "USDT");

    reportGenerator.printPoolDetails(
        "0xff970a61a04b1ca14834a43f5de4533ebddb5cc8",
        "0xfd086bc7cd5c481dcc9c85ebe478a1c0b69fcbb9", "USDC", "USDT");
}

void ArbitrageScanner::scanAllPairs()
{
    std::cout << "\n=== SCANNING ALL PAIRS FOR LIQUIDITY ===" << std::endl;

    auto tradingPairs = tokenManager.generateTradingPairs();
    int validPairs = 0;

    for (const auto& [tokenA, tokenB] : tradingPairs) {
        std::string nameA = tokenManager.getSymbol(tokenA);
        std::string nameB = tokenManager.getSymbol(tokenB);

        if (poolAnalyzer.hasValidPool(tokenA, tokenB)) {
            validPairs++;
            std::cout << "Active: " << nameA << "/" << nameB << std::endl;
        }
    }

    std::cout << "\nFound " << validPairs << " pairs with sufficient liquidity."
              << std::endl;
}

void ArbitrageScanner::performArbitrageAnalysis()
{
    std::cout << "\n=== TRIANGLE ARBITRAGE ANALYSIS ===" << std::endl;

    auto triangleCombinations = tokenManager.generateTriangleCombinations();
    auto opportunities
        = arbitrageAnalyzer.scanForArbitrageOpportunities(triangleCombinations);

    // 打印详细的套利机会
    for (const auto& opportunity : opportunities) {
        reportGenerator.printArbitrageOpportunity(opportunity);
    }

    // 打印总结报告
    reportGenerator.printSummaryReport(opportunities);
}

void ArbitrageScanner::printRecommendations()
{
    std::cout << "\n=== MONITORING RECOMMENDATIONS ===" << std::endl;
    std::cout << "For real-time arbitrage monitoring, focus on:" << std::endl;
    std::cout << "1. Stablecoin triangles (USDC/USDT/DAI) - Lower risk, "
                 "frequent opportunities"
              << std::endl;
    std::cout << "2. WETH-based triangles - Higher volume, better execution"
              << std::endl;
    std::cout << "3. Pools with >" << Config::MIN_LIQUIDITY
              << " liquidity - Reduced slippage impact" << std::endl;
    std::cout << "4. Fee tiers 0.05% and 0.3% - Most active pools" << std::endl;

    std::cout << "\nImportant Considerations:" << std::endl;
    std::cout << "- Slippage effects (can significantly reduce actual profits)"
              << std::endl;
    std::cout << "- Gas costs (typically $1-10 per transaction on Arbitrum)"
              << std::endl;
    std::cout << "- MEV competition (bots may front-run profitable trades)"
              << std::endl;
    std::cout << "- Price impact from large trades" << std::endl;
    std::cout << "- Network congestion and failed transactions" << std::endl;

    std::cout << "\nUpgrade Suggestions:" << std::endl;
    std::cout << "- Add real-time price monitoring" << std::endl;
    std::cout << "- Implement slippage calculation" << std::endl;
    std::cout << "- Add gas cost estimation" << std::endl;
    std::cout << "- Include more DEX protocols (SushiSwap, Curve, etc.)"
              << std::endl;
    std::cout << "- Add automated execution capabilities" << std::endl;
}
