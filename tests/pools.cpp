#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <cmath>
#include <httplib.h>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

using json = nlohmann::json;

// Convert large hex string to long double more accurately
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

int main()
{
    // Let's test with the actual sqrtPriceX96 value we extracted
    std::string sqrtPriceX96Hex = "349aafad6000000000000";

    std::cout << "Testing price calculation with actual sqrtPriceX96"
              << std::endl;
    std::cout << "================================================="
              << std::endl;
    std::cout << "sqrtPriceX96 hex: " << sqrtPriceX96Hex << std::endl;

    // Convert to long double
    long double sqrtPriceX96 = hexToLongDouble(sqrtPriceX96Hex);
    std::cout << "sqrtPriceX96 decimal: " << std::scientific
              << std::setprecision(15) << sqrtPriceX96 << std::endl;

    // Calculate price: (sqrtPriceX96 / 2^96)^2
    long double Q96 = powl(2.0L, 96.0L);
    std::cout << "Q96 (2^96): " << std::scientific << Q96 << std::endl;

    long double sqrtPrice = sqrtPriceX96 / Q96;
    std::cout << "sqrtPrice: " << std::scientific << sqrtPrice << std::endl;

    long double rawPrice = sqrtPrice * sqrtPrice;
    std::cout << "Raw price (token1/token0): " << std::scientific << rawPrice
              << std::endl;

    // Adjust for decimals: WETH (18 decimals) vs USDC (6 decimals)
    // Price adjustment = 10^(decimals0 - decimals1) = 10^(18-6) = 10^12
    long double decimalAdjustment = powl(10.0L, 12.0L);
    long double finalPrice = rawPrice * decimalAdjustment;

    std::cout << "Decimal adjustment: " << std::scientific << decimalAdjustment
              << std::endl;
    std::cout << "Final price (USDC per WETH): " << std::fixed
              << std::setprecision(2) << finalPrice << std::endl;

    // Let's also try a different approach - maybe the issue is with decimal
    // adjustment
    std::cout << "\n=== Alternative calculation ===" << std::endl;

    // What if we don't apply decimal adjustment?
    std::cout << "Price without decimal adjustment: " << std::scientific
              << rawPrice << std::endl;

    // What if decimal adjustment is wrong direction?
    long double altPrice1 = rawPrice / decimalAdjustment;
    std::cout << "Price with inverse decimal adjustment: " << std::scientific
              << altPrice1 << std::endl;

    // What if it's 10^6 instead of 10^12?
    long double altPrice2 = rawPrice * powl(10.0L, 6.0L);
    std::cout << "Price with 10^6 adjustment: " << std::fixed
              << std::setprecision(8) << altPrice2 << std::endl;

    // What if it's 10^-6?
    long double altPrice3 = rawPrice / powl(10.0L, 6.0L);
    std::cout << "Price with 10^-6 adjustment: " << std::fixed
              << std::setprecision(8) << altPrice3 << std::endl;

    // Let me try the reverse: maybe token0 is USDC and token1 is WETH?
    std::cout << "\n=== If token order is reversed ===" << std::endl;
    long double reversedPrice = 1.0L / finalPrice;
    std::cout << "Reversed price: " << std::fixed << std::setprecision(8)
              << reversedPrice << std::endl;

    return 0;
}
