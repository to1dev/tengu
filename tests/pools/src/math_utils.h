#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <cstdint>
#include <string>

class MathUtils {
public:
    static std::string toLowerCase(const std::string& str);
    static long double hexToLongDouble(const std::string& hexStr);
    static long double sqrtPriceX96ToPrice(const std::string& sqrtPriceX96Hex,
        uint8_t decimals0, uint8_t decimals1);
    static std::string formatHighPrecision(
        long double value, int maxDecimals = 12);
};

#endif // MATH_UTILS_H
