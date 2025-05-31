#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include <string>
#include <vector>

class Config {
public:
    // RPC配置
    static const std::string RPC_HOST;
    static const std::string RPC_PATH;
    static const std::string UNISWAP_V3_FACTORY;

    // 费率配置
    static const std::vector<uint32_t> STANDARD_FEES;
    static const uint64_t MIN_LIQUIDITY;
    static const long double MIN_ARBITRAGE_PROFIT;

    // 函数签名
    static const std::string GET_POOL_FUNC;
    static const std::string LIQUIDITY_FUNC;
    static const std::string SLOT0_FUNC;
    static const std::string TOKEN0_FUNC;
    static const std::string TOKEN1_FUNC;
    static const std::string FEE_FUNC;
};

#endif // CONFIG_H
