#ifndef ERRORS_H
#define ERRORS_H

#include <stdexcept>
#include <string>

namespace Daitengu::Wallets {

class WalletException : public std::runtime_error {
public:
    explicit WalletException(const std::string& msg)
        : std::runtime_error("WalletException: " + msg)
    {
    }
};

class MnemonicException : public WalletException {
public:
    explicit MnemonicException(const std::string& msg)
        : WalletException("Mnemonic: " + msg)
    {
    }
};

class DatabaseException : public WalletException {
public:
    explicit DatabaseException(const std::string& msg)
        : WalletException("Database: " + msg)
    {
    }
};

}

#endif // ERRORS_H
