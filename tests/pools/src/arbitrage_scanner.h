#ifndef ARBITRAGE_SCANNER_H
#define ARBITRAGE_SCANNER_H

#include "arbitrage_analyzer.h"
#include "pool_analyzer.h"
#include "report_generator.h"
#include "token_manager.h"

class ArbitrageScanner {
private:
    TokenManager tokenManager;
    PoolAnalyzer poolAnalyzer;
    ArbitrageAnalyzer arbitrageAnalyzer;
    ReportGenerator reportGenerator;

    void printHeader();
    void printConfiguration();
    void analyzeMajorPools();
    void scanAllPairs();
    void performArbitrageAnalysis();
    void printRecommendations();

public:
    ArbitrageScanner();
    void run();
};

#endif // ARBITRAGE_SCANNER_H
