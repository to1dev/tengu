// g++ -std=c++20 -o get_price get_price.cpp

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int main()
{
    std::string hex_str = "de85dd549e9803000100000000000000";

    // Try to convert the hexadecimal string to a decimal integer (may truncate)
    uint64_t val_int;
    try {
        val_int = std::stoull(hex_str, nullptr, 16);
        std::cout << "As a decimal large integer: " << val_int << std::endl;
    } catch (const std::exception& e) {
        std::cout << "As a decimal large integer: Number too large, cannot "
                     "convert directly"
                  << std::endl;
    }

    // Convert hexadecimal string to byte array
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hex_str.length(); i += 2) {
        std::string byte_str = hex_str.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoi(byte_str, nullptr, 16));
        bytes.push_back(byte);
    }

    // Reverse byte order (little-endian to big-endian)
    // For little-endian storage, we need to reverse it to big-endian for
    // parsing
    std::vector<uint8_t> bytes_reversed = bytes;
    std::reverse(bytes_reversed.begin(), bytes_reversed.end());

    // For 128-bit integers, we use two 64-bit integers (high bits and low bits)
    uint64_t high_bits = 0;
    uint64_t low_bits = 0;

    // Fill high_bits (first 8 bytes of the reversed array)
    for (size_t i = 0; i < std::min(size_t(8), bytes_reversed.size()); ++i) {
        high_bits = (high_bits << 8) | bytes_reversed[i];
    }

    // Fill low_bits (last 8 bytes of the reversed array)
    for (size_t i = 8; i < std::min(size_t(16), bytes_reversed.size()); ++i) {
        low_bits = (low_bits << 8) | bytes_reversed[i];
    }

    std::cout << low_bits << std::endl;

    // Calculate sqrtPrice (using long double for better precision)
    const long double Q64_64 = std::powl(2.0L, 64); // 2^64
    long double sqrt_price = static_cast<long double>(high_bits)
        + static_cast<long double>(low_bits) / Q64_64;

    std::cout << "sqrtPrice (as floating point) = " << std::setprecision(17)
              << sqrt_price << std::endl;

    // Calculate price
    long double price = sqrt_price * sqrt_price;
    std::cout << "price = " << std::setprecision(17) << price << std::endl;

    return 0;
}
