#pragma once

#include <map>
#include <tuple>
#include <vector>

#include "types.h"

class TokenManager {
private:
    std::map<std::string, Token> tokenMap;
    std::vector<std::string> majorTokens;
    std::vector<std::string> stableTokens;
    std::vector<std::string> defiTokens;

    void initializeTokens();

public:
    TokenManager();

    const Token* getToken(const std::string& address) const;
    std::string getSymbol(const std::string& address) const;
    uint8_t getDecimals(const std::string& address) const;
    bool isStablecoin(const std::string& address) const;

    std::vector<std::pair<std::string, std::string>>
    generateTradingPairs() const;
    std::vector<std::tuple<std::string, std::string, std::string>>
    generateTriangleCombinations() const;

    size_t getTokenCount() const;

    // 添加新代币的接口
    void addToken(const std::string& address, const std::string& symbol,
        uint8_t decimals, bool isStable = false);
};
