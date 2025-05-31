#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <algorithm>
#include <cmath>
#include <httplib.h>
#include <iomanip>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using json = nlohmann::json;

const std::string RPC_HOST = "arb1.arbitrum.io";
const std::string RPC_PATH = "/rpc";
const std::string UNISWAP_V3_FACTORY
    = "0x1F98431c8aD98523631AE4a59f267346ea31F984";

// Function signatures
const std::string GET_POOL_FUNC = "0x1698ee82";
const std::string LIQUIDITY_FUNC = "0x1a686502";
const std::string SLOT0_FUNC = "0x3850c7bd";
const std::string TOKEN0_FUNC = "0x0dfe1681";
const std::string TOKEN1_FUNC = "0xd21220a7";

const std::vector<uint32_t> STANDARD_FEES = { 100, 500, 3000, 10000 };

// 最小流动性要求 (过滤掉流动性太低的池子)
const uint64_t MIN_LIQUIDITY = 1000000; // 1M liquidity units

struct PoolInfo {
    std::string address;
    uint32_t fee;
    uint64_t liquidity;
    double price;
    std::string token0;
    std::string token1;
    bool valid;
};

struct Token {
    std::string address;
    std::string symbol;
    uint8_t decimals;
    bool isStablecoin;
};

// =====================================
// 代币配置区域 - 在这里添加新代币
// =====================================
std::map<std::string, Token> tokenMap = {
    // 主要代币
    { "0x82af49447d8a07e3bd95bd0d56f35241523fbab1",
        { "0x82af49447d8a07e3bd95bd0d56f35241523fbab1", "WETH", 18, false } },

    // 稳定币
    { "0xff970a61a04b1ca14834a43f5de4533ebddb5cc8",
        { "0xff970a61a04b1ca14834a43f5de4533ebddb5cc8", "USDC", 6, true } },
    { "0xfd086bc7cd5c481dcc9c85ebe478a1c0b69fcbb9",
        { "0xfd086bc7cd5c481dcc9c85ebe478a1c0b69fcbb9", "USDT", 6, true } },
    { "0xda10009cbd5d07dd0cecc66161fc93d7c9000da1",
        { "0xda10009cbd5d07dd0cecc66161fc93d7c9000da1", "DAI", 18, true } },

    // Layer 1 代币
    { "0x2f2a2543b76a4166549f7aab2e75bef0aefc5b0f",
        { "0x2f2a2543b76a4166549f7aab2e75bef0aefc5b0f", "WBTC", 8, false } },

    // Arbitrum 生态代币
    { "0x912ce59144191c1204e64559fe8253a0e49e6548",
        { "0x912ce59144191c1204e64559fe8253a0e49e6548", "ARB", 18, false } },
    { "0xfc5a1a6eb076a2c7ad06ed22c90d7e710e35ad0a",
        { "0xfc5a1a6eb076a2c7ad06ed22c90d7e710e35ad0a", "GMX", 18, false } },

    // DeFi 代币
    { "0xf97f4df75117a78c1a5a0dbb814af92458539fb4",
        { "0xf97f4df75117a78c1a5a0dbb814af92458539fb4", "LINK", 18, false } },
    { "0x080f6aed32fc474dd5717105dba5ea57268f46eb",
        { "0x080f6aed32fc474dd5717105dba5ea57268f46eb", "SYN", 18, false } },

    // Meme 代币 (高波动性)
    { "0x4d15a3a2286d883af0aa1b3f21367843fac63e07",
        { "0x4d15a3a2286d883af0aa1b3f21367843fac63e07", "SUSHI", 18, false } },
};

// 代币分组 - 用于生成交易对
std::vector<std::string> MAJOR_TOKENS = {
    "0x82af49447d8a07e3bd95bd0d56f35241523fbab1", // WETH
    "0x2f2a2543b76a4166549f7aab2e75bef0aefc5b0f", // WBTC
};

std::vector<std::string> STABLE_TOKENS = {
    "0xff970a61a04b1ca14834a43f5de4533ebddb5cc8", // USDC
    "0xfd086bc7cd5c481dcc9c85ebe478a1c0b69fcbb9", // USDT
    "0xda10009cbd5d07dd0cecc66161fc93d7c9000da1", // DAI
};

std::vector<std::string> DEFI_TOKENS = {
    "0x912ce59144191c1204e64559fe8253a0e49e6548", // ARB
    "0xfc5a1a6eb076a2c7ad06ed22c90d7e710e35ad0a", // GMX
    "0xf97f4df75117a78c1a5a0dbb814af92458539fb4", // LINK
    "0x080f6aed32fc474dd5717105dba5ea57268f46eb", // SYN
    "0x4d15a3a2286d883af0aa1b3f21367843fac63e07", // SUSHI
};

// 生成所有可能的交易对
std::vector<std::pair<std::string, std::string>> generateTradingPairs()
{
    std::vector<std::pair<std::string, std::string>> pairs;
    std::set<std::pair<std::string, std::string>> uniquePairs;

    auto addPairs = [&](const std::vector<std::string>& group1,
                        const std::vector<std::string>& group2) {
        for (const auto& token1 : group1) {
            for (const auto& token2 : group2) {
                if (token1 != token2) {
                    // 确保地址顺序一致性
                    auto pair = token1 < token2
                        ? std::make_pair(token1, token2)
                        : std::make_pair(token2, token1);
                    uniquePairs.insert(pair);
                }
            }
        }
    };

    // 主要代币 vs 稳定币
    addPairs(MAJOR_TOKENS, STABLE_TOKENS);

    // 主要代币 vs DeFi代币
    addPairs(MAJOR_TOKENS, DEFI_TOKENS);

    // 稳定币 vs DeFi代币
    addPairs(STABLE_TOKENS, DEFI_TOKENS);

    // 稳定币之间 (套利机会较多)
    addPairs(STABLE_TOKENS, STABLE_TOKENS);

    // DeFi代币之间
    addPairs(DEFI_TOKENS, DEFI_TOKENS);

    // 转换为vector
    for (const auto& pair : uniquePairs) {
        pairs.push_back(pair);
    }

    return pairs;
}

// 生成三角套利组合
std::vector<std::tuple<std::string, std::string, std::string>>
generateTriangleCombinations()
{
    std::vector<std::tuple<std::string, std::string, std::string>> triangles;

    // 高频套利组合: 主要代币 + 两个稳定币
    for (const auto& major : MAJOR_TOKENS) {
        for (size_t i = 0; i < STABLE_TOKENS.size(); i++) {
            for (size_t j = i + 1; j < STABLE_TOKENS.size(); j++) {
                triangles.push_back(
                    std::make_tuple(major, STABLE_TOKENS[i], STABLE_TOKENS[j]));
            }
        }
    }

    // 主要代币 + 稳定币 + DeFi代币
    for (const auto& major : MAJOR_TOKENS) {
        for (const auto& stable : STABLE_TOKENS) {
            for (const auto& defi : DEFI_TOKENS) {
                triangles.push_back(std::make_tuple(major, stable, defi));
            }
        }
    }

    // 高波动性组合: 三个DeFi代币
    for (size_t i = 0; i < DEFI_TOKENS.size(); i++) {
        for (size_t j = i + 1; j < DEFI_TOKENS.size(); j++) {
            for (size_t k = j + 1; k < DEFI_TOKENS.size(); k++) {
                triangles.push_back(std::make_tuple(
                    DEFI_TOKENS[i], DEFI_TOKENS[j], DEFI_TOKENS[k]));
            }
        }
    }

    return triangles;
}

// =====================================
// 核心功能函数
// =====================================

std::string toLowerCase(const std::string& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

long double hexToLongDouble(const std::string& hexStr)
{
    long double result = 0.0L;
    for (char c : hexStr) {
        result *= 16.0L;
        if (c >= '0' && c <= '9') {
            result += (c - '0');
        } else if (c >= 'a' && c <= 'f') {
            result += (c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            result += (c - 'A' + 10);
        }
    }
    return result;
}

double sqrtPriceX96ToPrice(
    const std::string& sqrtPriceX96Hex, uint8_t decimals0, uint8_t decimals1)
{
    try {
        long double sqrtPriceX96 = hexToLongDouble(sqrtPriceX96Hex);
        if (sqrtPriceX96 == 0.0L)
            return 0.0;

        long double Q96 = powl(2.0L, 96.0L);
        long double sqrtPrice = sqrtPriceX96 / Q96;
        long double rawPrice = sqrtPrice * sqrtPrice;

        long double decimalAdjustment = powl(10.0L, decimals0 - decimals1);
        long double finalPrice = rawPrice * decimalAdjustment;

        return (double)finalPrice;
    } catch (const std::exception& e) {
        return 0.0;
    }
}

json sendRpcRequest(const std::string& method, const json& params)
{
    httplib::SSLClient cli(RPC_HOST);
    cli.enable_server_certificate_verification(false);
    cli.set_connection_timeout(30, 0);

    json request = { { "jsonrpc", "2.0" }, { "method", method },
        { "params", params }, { "id", 1 } };

    auto res = cli.Post(RPC_PATH, request.dump(), "application/json");

    if (!res || res->status != 200) {
        throw std::runtime_error("RPC request failed");
    }

    auto response = json::parse(res->body);
    if (response.contains("error")) {
        throw std::runtime_error("RPC error: " + response["error"].dump());
    }

    return response["result"];
}

json callContract(const std::string& contractAddress, const std::string& data)
{
    json params = json::array();
    params.push_back({ { "to", contractAddress }, { "data", data } });
    params.push_back("latest");

    return sendRpcRequest("eth_call", params);
}

std::string getPoolAddress(
    const std::string& tokenA, const std::string& tokenB, uint32_t fee)
{
    std::stringstream feeHex;
    feeHex << std::hex << std::setfill('0') << std::setw(8) << fee;

    std::string data = GET_POOL_FUNC + std::string(24, '0') + tokenA.substr(2)
        + std::string(24, '0') + tokenB.substr(2) + std::string(56, '0')
        + feeHex.str();

    json result = callContract(UNISWAP_V3_FACTORY, data);
    std::string poolAddress = result.get<std::string>();

    if (poolAddress.length() >= 42) {
        std::string realAddress
            = "0x" + poolAddress.substr(poolAddress.length() - 40);
        if (realAddress != "0x0000000000000000000000000000000000000000") {
            return toLowerCase(realAddress);
        }
    }
    return "";
}

uint64_t getPoolLiquidity(const std::string& poolAddress)
{
    try {
        json result = callContract(poolAddress, LIQUIDITY_FUNC);
        std::string liquidityHex = result.get<std::string>();
        if (liquidityHex.length() > 2) {
            return std::stoull(liquidityHex.substr(2), nullptr, 16);
        }
    } catch (...) {
    }
    return 0;
}

std::pair<std::string, std::string> getPoolTokens(
    const std::string& poolAddress)
{
    try {
        json token0Result = callContract(poolAddress, TOKEN0_FUNC);
        json token1Result = callContract(poolAddress, TOKEN1_FUNC);

        std::string token0Hex = token0Result.get<std::string>();
        std::string token1Hex = token1Result.get<std::string>();

        std::string token0
            = toLowerCase("0x" + token0Hex.substr(token0Hex.length() - 40));
        std::string token1
            = toLowerCase("0x" + token1Hex.substr(token1Hex.length() - 40));

        return { token0, token1 };
    } catch (const std::exception& e) {
        return { "", "" };
    }
}

std::string extractSqrtPriceX96(const std::string& slot0Hex)
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

// 改进的getBestPool函数 - 添加流动性过滤和更好的匹配逻辑
PoolInfo getBestPool(const std::string& tokenA, const std::string& tokenB)
{
    PoolInfo bestPool = { "", 0, 0, 0.0, "", "", false };
    uint64_t maxLiquidity = 0;

    // 尝试两个方向的代币顺序
    std::vector<std::pair<std::string, std::string>> tokenPairs
        = { { tokenA, tokenB }, { tokenB, tokenA } };

    for (const auto& [tA, tB] : tokenPairs) {
        for (uint32_t fee : STANDARD_FEES) {
            try {
                std::string poolAddress = getPoolAddress(tA, tB, fee);

                if (!poolAddress.empty()) {
                    uint64_t liquidity = getPoolLiquidity(poolAddress);

                    // 流动性过滤
                    if (liquidity < MIN_LIQUIDITY) {
                        continue;
                    }

                    auto [token0, token1] = getPoolTokens(poolAddress);

                    json result = callContract(poolAddress, SLOT0_FUNC);
                    std::string slot0Hex = result.get<std::string>();
                    std::string sqrtPriceX96Hex = extractSqrtPriceX96(slot0Hex);

                    if (!sqrtPriceX96Hex.empty() && sqrtPriceX96Hex != "0") {
                        uint8_t decimals0 = 18, decimals1 = 18;

                        if (tokenMap.find(token0) != tokenMap.end()) {
                            decimals0 = tokenMap[token0].decimals;
                        }
                        if (tokenMap.find(token1) != tokenMap.end()) {
                            decimals1 = tokenMap[token1].decimals;
                        }

                        double price = sqrtPriceX96ToPrice(
                            sqrtPriceX96Hex, decimals0, decimals1);

                        if (liquidity > maxLiquidity && price > 0) {
                            bestPool = { poolAddress, fee, liquidity, price,
                                token0, token1, true };
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

void queryTokenPairPools(const std::string& tokenA, const std::string& tokenB,
    const std::string& tokenAName, const std::string& tokenBName,
    bool showDetails = true)
{
    if (showDetails) {
        std::cout << "\n=== " << tokenAName << "/" << tokenBName
                  << " Pools ===" << std::endl;
    }

    bool foundAnyPool = false;

    for (uint32_t fee : STANDARD_FEES) {
        try {
            std::string poolAddress = getPoolAddress(tokenA, tokenB, fee);

            if (!poolAddress.empty()) {
                uint64_t liquidity = getPoolLiquidity(poolAddress);

                // 只显示有足够流动性的池子
                if (liquidity < MIN_LIQUIDITY) {
                    continue;
                }

                foundAnyPool = true;

                if (showDetails) {
                    std::cout << "\nPool Address: " << poolAddress << std::endl;
                    std::cout << "Fee Tier: " << fee / 10000.0 << "% (" << fee
                              << " bps)" << std::endl;
                    std::cout << "Liquidity: " << liquidity << std::endl;
                }

                auto [token0, token1] = getPoolTokens(poolAddress);
                std::string symbol0 = "Unknown";
                std::string symbol1 = "Unknown";

                if (tokenMap.find(token0) != tokenMap.end()) {
                    symbol0 = tokenMap[token0].symbol;
                }
                if (tokenMap.find(token1) != tokenMap.end()) {
                    symbol1 = tokenMap[token1].symbol;
                }

                if (showDetails) {
                    std::cout << "Pool tokens: " << symbol0 << " (token0) / "
                              << symbol1 << " (token1)" << std::endl;
                }

                json result = callContract(poolAddress, SLOT0_FUNC);
                std::string slot0Hex = result.get<std::string>();
                std::string sqrtPriceX96Hex = extractSqrtPriceX96(slot0Hex);

                if (!sqrtPriceX96Hex.empty() && sqrtPriceX96Hex != "0") {
                    uint8_t decimals0 = 18, decimals1 = 18;

                    if (tokenMap.find(token0) != tokenMap.end()) {
                        decimals0 = tokenMap[token0].decimals;
                    }
                    if (tokenMap.find(token1) != tokenMap.end()) {
                        decimals1 = tokenMap[token1].decimals;
                    }

                    double price = sqrtPriceX96ToPrice(
                        sqrtPriceX96Hex, decimals0, decimals1);

                    if (showDetails) {
                        std::cout << "Price: 1 " << symbol0 << " = "
                                  << std::fixed << std::setprecision(4) << price
                                  << " " << symbol1 << std::endl;
                        std::cout << "Price: 1 " << symbol1 << " = "
                                  << std::fixed << std::setprecision(6)
                                  << (1.0 / price) << " " << symbol0
                                  << std::endl;
                        std::cout << "----------------------------------------"
                                  << std::endl;
                    }
                }
            }
        } catch (const std::exception& e) {
            if (showDetails) {
                std::cout << "Error checking fee " << fee / 10000.0
                          << "%: " << e.what() << std::endl;
            }
        }
    }

    if (!foundAnyPool && showDetails) {
        std::cout
            << "No pools with sufficient liquidity found for this token pair"
            << std::endl;
    }
}

void checkTriangleArbitrage(const std::string& tokenA,
    const std::string& tokenB, const std::string& tokenC,
    const std::string& nameA, const std::string& nameB,
    const std::string& nameC)
{
    PoolInfo poolAB = getBestPool(tokenA, tokenB);
    PoolInfo poolBC = getBestPool(tokenB, tokenC);
    PoolInfo poolCA = getBestPool(tokenC, tokenA);

    if (!poolAB.valid || !poolBC.valid || !poolCA.valid) {
        return; // 静默跳过无效的三角组合
    }

    std::cout << "\n============================================" << std::endl;
    std::cout << "Triangle Arbitrage: " << nameA << " -> " << nameB << " -> "
              << nameC << " -> " << nameA << std::endl;
    std::cout << "============================================" << std::endl;

    std::cout << "Triangle pools found:" << std::endl;
    std::cout << nameA << " <-> " << nameB << ": " << poolAB.address
              << " (Fee: " << poolAB.fee / 10000.0
              << "%, Liq: " << poolAB.liquidity << ")" << std::endl;
    std::cout << nameB << " <-> " << nameC << ": " << poolBC.address
              << " (Fee: " << poolBC.fee / 10000.0
              << "%, Liq: " << poolBC.liquidity << ")" << std::endl;
    std::cout << nameC << " <-> " << nameA << ": " << poolCA.address
              << " (Fee: " << poolCA.fee / 10000.0
              << "%, Liq: " << poolCA.liquidity << ")" << std::endl;

    // 计算汇率
    double rateAB, rateBC, rateCA;

    if (toLowerCase(tokenA) == poolAB.token0) {
        rateAB = poolAB.price;       // B/A
    } else {
        rateAB = 1.0 / poolAB.price; // B/A
    }

    if (toLowerCase(tokenB) == poolBC.token0) {
        rateBC = poolBC.price;       // C/B
    } else {
        rateBC = 1.0 / poolBC.price; // C/B
    }

    if (toLowerCase(tokenC) == poolCA.token0) {
        rateCA = poolCA.price;       // A/C
    } else {
        rateCA = 1.0 / poolCA.price; // A/C
    }

    std::cout << "\nExchange rates:" << std::endl;
    std::cout << "1 " << nameA << " -> " << std::fixed << std::setprecision(4)
              << rateAB << " " << nameB << std::endl;
    std::cout << "1 " << nameB << " -> " << std::fixed << std::setprecision(4)
              << rateBC << " " << nameC << std::endl;
    std::cout << "1 " << nameC << " -> " << std::fixed << std::setprecision(4)
              << rateCA << " " << nameA << std::endl;

    double expectedReturn = rateAB * rateBC * rateCA;
    double feeMultiplier = (1.0 - poolAB.fee / 1000000.0)
        * (1.0 - poolBC.fee / 1000000.0) * (1.0 - poolCA.fee / 1000000.0);
    double netReturn = expectedReturn * feeMultiplier;
    double profitPercent = (netReturn - 1.0) * 100.0;

    std::cout << "\nArbitrage Analysis:" << std::endl;
    std::cout << "Expected return (before fees): " << std::fixed
              << std::setprecision(6) << expectedReturn << std::endl;
    std::cout << "Net return (after fees): " << std::fixed
              << std::setprecision(6) << netReturn << std::endl;
    std::cout << "Profit percentage: " << std::fixed << std::setprecision(4)
              << profitPercent << "%" << std::endl;

    if (std::abs(profitPercent) > 0.01) {
        if (profitPercent > 0) {
            std::cout << "\n*** ARBITRAGE OPPORTUNITY DETECTED! ***"
                      << std::endl;
            std::cout << "Direction: " << nameA << " -> " << nameB << " -> "
                      << nameC << " -> " << nameA << std::endl;
            std::cout << "Estimated profit: " << profitPercent << "%"
                      << std::endl;
        } else {
            std::cout << "\n*** REVERSE ARBITRAGE OPPORTUNITY ***" << std::endl;
            std::cout << "Direction: " << nameA << " -> " << nameC << " -> "
                      << nameB << " -> " << nameA << std::endl;
            std::cout << "Estimated profit: " << std::fixed
                      << std::setprecision(4) << -profitPercent << "%"
                      << std::endl;
        }
    } else {
        std::cout << "\nNo significant arbitrage opportunity found."
                  << std::endl;
    }

    uint64_t minLiquidity
        = std::min({ poolAB.liquidity, poolBC.liquidity, poolCA.liquidity });
    std::cout << "Minimum liquidity in path: " << minLiquidity << std::endl;
}

int main()
{
    std::cout << "Advanced Uniswap V3 Pool & Arbitrage Scanner" << std::endl;
    std::cout << "===========================================" << std::endl;

    // 显示配置信息
    std::cout << "Configuration:" << std::endl;
    std::cout << "- Minimum liquidity requirement: " << MIN_LIQUIDITY
              << std::endl;
    std::cout << "- Total tokens configured: " << tokenMap.size() << std::endl;
    std::cout << "- Scanning Arbitrum network" << std::endl;

    // 生成交易对
    auto tradingPairs = generateTradingPairs();
    std::cout << "- Generated " << tradingPairs.size() << " trading pairs"
              << std::endl;

    // 查询主要池子信息
    std::cout << "\n=== MAJOR POOLS ANALYSIS ===" << std::endl;

    // 只显示主要的几个池子详情
    queryTokenPairPools("0x82af49447d8a07e3bd95bd0d56f35241523fbab1",
        "0xff970a61a04b1ca14834a43f5de4533ebddb5cc8", "WETH", "USDC");
    queryTokenPairPools("0x82af49447d8a07e3bd95bd0d56f35241523fbab1",
        "0xfd086bc7cd5c481dcc9c85ebe478a1c0b69fcbb9", "WETH", "USDT");
    queryTokenPairPools("0xff970a61a04b1ca14834a43f5de4533ebddb5cc8",
        "0xfd086bc7cd5c481dcc9c85ebe478a1c0b69fcbb9", "USDC", "USDT");

    // 快速扫描所有其他池子(不显示详情)
    std::cout << "\n=== SCANNING ALL PAIRS FOR LIQUIDITY ===" << std::endl;
    int validPairs = 0;
    for (const auto& [tokenA, tokenB] : tradingPairs) {
        std::string nameA = tokenMap.find(tokenA) != tokenMap.end()
            ? tokenMap[tokenA].symbol
            : "Unknown";
        std::string nameB = tokenMap.find(tokenB) != tokenMap.end()
            ? tokenMap[tokenB].symbol
            : "Unknown";

        // 静默检查是否有有效池子
        bool hasValidPool = false;
        for (uint32_t fee : STANDARD_FEES) {
            try {
                std::string poolAddress = getPoolAddress(tokenA, tokenB, fee);
                if (!poolAddress.empty()) {
                    uint64_t liquidity = getPoolLiquidity(poolAddress);
                    if (liquidity >= MIN_LIQUIDITY) {
                        hasValidPool = true;
                        break;
                    }
                }
            } catch (...) {
                continue;
            }
        }

        if (hasValidPool) {
            validPairs++;
            std::cout << "* " << nameA << "/" << nameB
                      << " - Active pools found" << std::endl;
        }
    }

    std::cout << "\nFound " << validPairs << " pairs with sufficient liquidity."
              << std::endl;

    // 三角套利分析
    std::cout << "\n=== TRIANGLE ARBITRAGE ANALYSIS ===" << std::endl;

    auto triangleCombinations = generateTriangleCombinations();
    std::cout << "Analyzing " << triangleCombinations.size()
              << " triangle combinations..." << std::endl;

    int opportunitiesFound = 0;

    for (const auto& [tokenA, tokenB, tokenC] : triangleCombinations) {
        std::string nameA = tokenMap.find(tokenA) != tokenMap.end()
            ? tokenMap[tokenA].symbol
            : "Unknown";
        std::string nameB = tokenMap.find(tokenB) != tokenMap.end()
            ? tokenMap[tokenB].symbol
            : "Unknown";
        std::string nameC = tokenMap.find(tokenC) != tokenMap.end()
            ? tokenMap[tokenC].symbol
            : "Unknown";

        // 检查是否所有三个池子都有足够流动性
        PoolInfo poolAB = getBestPool(tokenA, tokenB);
        PoolInfo poolBC = getBestPool(tokenB, tokenC);
        PoolInfo poolCA = getBestPool(tokenC, tokenA);

        if (poolAB.valid && poolBC.valid && poolCA.valid) {
            opportunitiesFound++;
            checkTriangleArbitrage(tokenA, tokenB, tokenC, nameA, nameB, nameC);
        }
    }

    if (opportunitiesFound == 0) {
        std::cout
            << "\nNo complete triangle paths found with sufficient liquidity."
            << std::endl;
    } else {
        std::cout << "\nAnalyzed " << opportunitiesFound
                  << " complete triangle paths." << std::endl;
    }

    // 实时监控建议
    std::cout << "\n=== MONITORING RECOMMENDATIONS ===" << std::endl;
    std::cout << "For real-time arbitrage monitoring, focus on:" << std::endl;
    std::cout << "1. Stablecoin triangles (USDC/USDT/DAI) - Lower risk, "
                 "frequent opportunities"
              << std::endl;
    std::cout << "2. WETH-based triangles - Higher volume, better execution"
              << std::endl;
    std::cout << "3. Pools with >1M liquidity - Reduced slippage impact"
              << std::endl;
    std::cout << "4. Fee tiers 0.05% and 0.3% - Most active pools" << std::endl;

    std::cout << "\nNote: This analysis does not account for:" << std::endl;
    std::cout << "- Slippage effects (can significantly reduce actual profits)"
              << std::endl;
    std::cout << "- Gas costs (typically $1-10 per transaction on Arbitrum)"
              << std::endl;
    std::cout << "- MEV competition (bots may front-run profitable trades)"
              << std::endl;
    std::cout << "- Price impact from large trades" << std::endl;
    std::cout << "- Network congestion and failed transactions" << std::endl;

    return 0;
}
