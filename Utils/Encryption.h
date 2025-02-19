#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <cstring>
#include <format>
#include <stdexcept>
#include <vector>

#include <QString>

#include <sodium.h>
#include <xxhash.h>

#include "Security/Security.h"

using namespace Daitengu::Security;

namespace Daitengu::Utils {

inline constexpr int DEFAULT_EASYHASH_SEED = 6978;

class Encryption {
public:
    Encryption();
    virtual ~Encryption() = default;

    static bool init();

    static std::string genRandomHash();
    static QString easyHash(
        const QString& input, unsigned long long seed = DEFAULT_EASYHASH_SEED);
    static std::string easyHash(const std::string& input,
        unsigned long long seed = DEFAULT_EASYHASH_SEED);
    static std::string generateStrongPassword(size_t length);

    static QString encryptText(
        const QString& plainText, const QString& password = defaultKey);
    static std::string encryptText(const std::string& plainText,
        const std::string& password = defaultKey.toStdString());

    static QString decryptText(
        const QString& encodedText, const QString& password = defaultKey);
    static std::string decryptText(const std::string& encodedText,
        const std::string& password = defaultKey.toStdString());
};

}
#endif // ENCRYPTION_H
