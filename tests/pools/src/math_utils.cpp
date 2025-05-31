#include "math_utils.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

std::string MathUtils::toLowerCase(const std::string& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

long double MathUtils::hexToLongDouble(const std::string& hexStr)
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

long double MathUtils::sqrtPriceX96ToPrice(
    const std::string& sqrtPriceX96Hex, uint8_t decimals0, uint8_t decimals1)
{
    try {
        long double sqrtPriceX96 = hexToLongDouble(sqrtPriceX96Hex);
        if (sqrtPriceX96 == 0.0L)
            return 0.0L;

        long double Q96 = powl(2.0L, 96.0L);
        long double sqrtPrice = sqrtPriceX96 / Q96;
        long double rawPrice = sqrtPrice * sqrtPrice;

        long double decimalAdjustment = powl(10.0L, decimals0 - decimals1);
        long double finalPrice = rawPrice * decimalAdjustment;

        return finalPrice;
    } catch (const std::exception& e) {
        return 0.0L;
    }
}

std::string MathUtils::formatHighPrecision(long double value, int maxDecimals)
{
    if (value == 0.0L)
        return "0";

    std::ostringstream oss;

    if (std::abs(value) < 1e-6L) {
        oss << std::scientific << std::setprecision(6) << value;
    } else if (std::abs(value) > 1e6L) {
        oss << std::fixed << std::setprecision(2) << value;
    } else {
        int precision;
        if (std::abs(value) >= 1.0L) {
            precision = std::min(6, maxDecimals);
        } else if (std::abs(value) >= 0.01L) {
            precision = std::min(8, maxDecimals);
        } else {
            precision = maxDecimals;
        }

        oss << std::fixed << std::setprecision(precision) << value;

        std::string result = oss.str();
        if (result.find('.') != std::string::npos) {
            result.erase(result.find_last_not_of('0') + 1, std::string::npos);
            result.erase(result.find_last_not_of('.') + 1, std::string::npos);
        }
        return result;
    }

    return oss.str();
}
