#include "token_manager.h"

#include <set>

TokenManager::TokenManager()
{
    initializeTokens();
}

void TokenManager::initializeTokens()
{
    // 主要代币
    tokenMap["0x82af49447d8a07e3bd95bd0d56f35241523fbab1"] = Token(
        "0x82af49447d8a07e3bd95bd0d56f35241523fbab1", "WETH", 18, false);
    tokenMap["0x2f2a2543b76a4166549f7aab2e75bef0aefc5b0f"]
        = Token("0x2f2a2543b76a4166549f7aab2e75bef0aefc5b0f", "WBTC", 8, false);

    // 稳定币
    tokenMap["0xff970a61a04b1ca14834a43f5de4533ebddb5cc8"]
        = Token("0xff970a61a04b1ca14834a43f5de4533ebddb5cc8", "USDC", 6, true);
    tokenMap["0xfd086bc7cd5c481dcc9c85ebe478a1c0b69fcbb9"]
        = Token("0xfd086bc7cd5c481dcc9c85ebe478a1c0b69fcbb9", "USDT", 6, true);
    tokenMap["0xda10009cbd5d07dd0cecc66161fc93d7c9000da1"]
        = Token("0xda10009cbd5d07dd0cecc66161fc93d7c9000da1", "DAI", 18, true);

    // DeFi代币
    tokenMap["0x912ce59144191c1204e64559fe8253a0e49e6548"]
        = Token("0x912ce59144191c1204e64559fe8253a0e49e6548", "ARB", 18, false);
    tokenMap["0xfc5a1a6eb076a2c7ad06ed22c90d7e710e35ad0a"]
        = Token("0xfc5a1a6eb076a2c7ad06ed22c90d7e710e35ad0a", "GMX", 18, false);
    tokenMap["0xf97f4df75117a78c1a5a0dbb814af92458539fb4"] = Token(
        "0xf97f4df75117a78c1a5a0dbb814af92458539fb4", "LINK", 18, false);
    tokenMap["0x080f6aed32fc474dd5717105dba5ea57268f46eb"]
        = Token("0x080f6aed32fc474dd5717105dba5ea57268f46eb", "SYN", 18, false);
    tokenMap["0x4d15a3a2286d883af0aa1b3f21367843fac63e07"] = Token(
        "0x4d15a3a2286d883af0aa1b3f21367843fac63e07", "SUSHI", 18, false);

    // 分组
    majorTokens = {
        "0x82af49447d8a07e3bd95bd0d56f35241523fbab1", // WETH
        "0x2f2a2543b76a4166549f7aab2e75bef0aefc5b0f"  // WBTC
    };

    stableTokens = {
        "0xff970a61a04b1ca14834a43f5de4533ebddb5cc8", // USDC
        "0xfd086bc7cd5c481dcc9c85ebe478a1c0b69fcbb9", // USDT
        "0xda10009cbd5d07dd0cecc66161fc93d7c9000da1"  // DAI
    };

    defiTokens = {
        "0x912ce59144191c1204e64559fe8253a0e49e6548", // ARB
        "0xfc5a1a6eb076a2c7ad06ed22c90d7e710e35ad0a", // GMX
        "0xf97f4df75117a78c1a5a0dbb814af92458539fb4", // LINK
        "0x080f6aed32fc474dd5717105dba5ea57268f46eb", // SYN
        "0x4d15a3a2286d883af0aa1b3f21367843fac63e07"  // SUSHI
    };
}

const Token* TokenManager::getToken(const std::string& address) const
{
    auto it = tokenMap.find(address);
    return (it != tokenMap.end()) ? &it->second : nullptr;
}

std::string TokenManager::getSymbol(const std::string& address) const
{
    const Token* token = getToken(address);
    return token ? token->symbol : "Unknown";
}

uint8_t TokenManager::getDecimals(const std::string& address) const
{
    const Token* token = getToken(address);
    return token ? token->decimals : 18;
}

bool TokenManager::isStablecoin(const std::string& address) const
{
    const Token* token = getToken(address);
    return token ? token->isStablecoin : false;
}

std::vector<std::pair<std::string, std::string>>
TokenManager::generateTradingPairs() const
{
    std::vector<std::pair<std::string, std::string>> pairs;
    std::set<std::pair<std::string, std::string>> uniquePairs;

    auto addPairs = [&](const std::vector<std::string>& group1,
                        const std::vector<std::string>& group2) {
        for (const auto& token1 : group1) {
            for (const auto& token2 : group2) {
                if (token1 != token2) {
                    auto pair = token1 < token2
                        ? std::make_pair(token1, token2)
                        : std::make_pair(token2, token1);
                    uniquePairs.insert(pair);
                }
            }
        }
    };

    addPairs(majorTokens, stableTokens);
    addPairs(majorTokens, defiTokens);
    addPairs(stableTokens, defiTokens);
    addPairs(stableTokens, stableTokens);
    addPairs(defiTokens, defiTokens);

    for (const auto& pair : uniquePairs) {
        pairs.push_back(pair);
    }

    return pairs;
}

std::vector<std::tuple<std::string, std::string, std::string>>
TokenManager::generateTriangleCombinations() const
{
    std::vector<std::tuple<std::string, std::string, std::string>> triangles;

    // 主要代币 + 两个稳定币 (高频套利)
    for (const auto& major : majorTokens) {
        for (size_t i = 0; i < stableTokens.size(); i++) {
            for (size_t j = i + 1; j < stableTokens.size(); j++) {
                triangles.push_back(
                    std::make_tuple(major, stableTokens[i], stableTokens[j]));
            }
        }
    }

    // 主要代币 + 稳定币 + DeFi代币
    for (const auto& major : majorTokens) {
        for (const auto& stable : stableTokens) {
            for (const auto& defi : defiTokens) {
                triangles.push_back(std::make_tuple(major, stable, defi));
            }
        }
    }

    return triangles;
}

size_t TokenManager::getTokenCount() const
{
    return tokenMap.size();
}

void TokenManager::addToken(const std::string& address,
    const std::string& symbol, uint8_t decimals, bool isStable)
{
    tokenMap[address] = Token(address, symbol, decimals, isStable);

    if (isStable) {
        stableTokens.push_back(address);
    } else {
        defiTokens.push_back(address);
    }
}
