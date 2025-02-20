#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <cstdint>
#include <cstring>
#include <iostream>
#include <random>
#include <regex>
#include <string>

#include <QLocale>
#include <QRegularExpression>
#include <QString>

namespace Daitengu::Utils {

inline int randomIndex(int start, int end)
{
    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<int> distribution(start, end);
    return distribution(generator);
}

inline std::string hideAddress(std::string_view address)
{
    if (address.length() <= 10) {
        return std::string(address);
    }

    return std::string(address.substr(0, 5)) + "..."
        + std::string(address.substr(address.size() - 5, 5));
}

inline QString hideAddress(const QString& address)
{
    if (address.length() <= 10) {
        return address;
    }

    return address.left(5) + "..." + address.right(5);
}

}
#endif // HELPERS_HPP
