#ifndef MNEMONIC_H
#define MNEMONIC_H

#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

#include "izanagi/bip39.h"

#ifdef __cplusplus
}
#endif

namespace Daitengu::Wallets {

class Mnemonic {
public:
    Mnemonic();

    [[nodiscard]] static std::string generate(int strength);
    [[nodiscard]] static std::string fromData(const std::vector<uint8_t>& data);
    static void clear();
    [[nodiscard]] static bool check(const std::string& mnemonic);
    [[nodiscard]] static std::vector<uint8_t> toBits(
        const std::string& mnemonic);
    [[nodiscard]] static std::vector<uint8_t> toSeed(
        const std::string& mnemonic, const std::string& passphrase = "",
        std::function<void(uint32_t, uint32_t)> progressCallback = nullptr);
    [[nodiscard]] static int findWord(const std::string& word);
    [[nodiscard]] static std::string completeWord(const std::string& prefix);
    [[nodiscard]] static std::string getWord(int index);
    [[nodiscard]] static uint32_t getWordCompletionMask(
        const std::string& prefix);
};

}
#endif // MNEMONIC_H
