#ifndef REPORT_GENERATOR_H
#define REPORT_GENERATOR_H

#include "pool_analyzer.h"
#include "token_manager.h"
#include "types.h"

#include <vector>

class ReportGenerator {
private:
    const TokenManager& tokenManager;
    PoolAnalyzer& poolAnalyzer;

    void printRiskAssessment(const ArbitrageOpportunity& opp);

public:
    ReportGenerator(const TokenManager& tm, PoolAnalyzer& pa);

    void printPoolDetails(const std::string& tokenA, const std::string& tokenB,
        const std::string& nameA, const std::string& nameB);

    void printArbitrageOpportunity(const ArbitrageOpportunity& opp);

    void printSummaryReport(
        const std::vector<ArbitrageOpportunity>& opportunities);
};

#endif // REPORT_GENERATOR_H
