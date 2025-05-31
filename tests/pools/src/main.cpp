#include "arbitrage_scanner.h"

#include <iostream>

int main()
{
    try {
        ArbitrageScanner scanner;
        scanner.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
