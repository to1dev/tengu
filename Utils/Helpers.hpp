#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <random>

namespace Daitengu::Utils {

inline int randomIndex(int start, int end)
{
    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<int> distribution(start, end);
    return distribution(generator);
}

}
#endif // HELPERS_HPP
