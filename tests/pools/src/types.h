#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <string>

struct Token {
    std::string address;
    std::string symbol;
    uint8_t decimals;
    bool isStablecoin;

    Token()
        : decimals(18)
        , isStablecoin(false)
    {
    }

    Token(const std::string& addr, const std::string& sym, uint8_t dec,
        bool stable)
        : address(addr)
        , symbol(sym)
        , decimals(dec)
        , isStablecoin(stable)
    {
    }
};

struct PoolInfo {
    std::string address;
    uint32_t searchFee;
    uint32_t actualFee;
    uint64_t liquidity;
    long double price;
    std::string token0;
    std::string token1;
    bool valid;
    bool feeMatched;

    PoolInfo()
        : searchFee(0)
        , actualFee(0)
        , liquidity(0)
        , price(0.0L)
        , valid(false)
        , feeMatched(false)
    {
    }
};

struct ArbitrageOpportunity {
    std::string tokenA, tokenB, tokenC;
    std::string nameA, nameB, nameC;
    PoolInfo poolAB, poolBC, poolCA;
    long double rateAB, rateBC, rateCA;
    long double expectedReturn;
    long double netReturn;
    long double profitPercent;
    uint64_t minLiquidity;
    bool isValid;

    ArbitrageOpportunity()
        : rateAB(0.0L)
        , rateBC(0.0L)
        , rateCA(0.0L)
        , expectedReturn(0.0L)
        , netReturn(0.0L)
        , profitPercent(0.0L)
        , minLiquidity(0)
        , isValid(false)
    {
    }
};

#endif // TYPES_H
