#ifndef BASEMNEMONIC_H
#define BASEMNEMONIC_H

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#include "Errors.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#include "izanagi/bip39.h"

#ifdef __cplusplus
}
#endif

namespace Daitengu::Wallets {

class BaseMnemonic {
public:
    explicit BaseMnemonic() = default;

    [[nodiscard]] static std::string generate(int strength = 128);
    [[nodiscard]] static std::string fromData(
        const std::vector<std::uint8_t>& data);
    [[nodiscard]] static bool check(const std::string& mnemonic);
    [[nodiscard]] static std::vector<std::uint8_t> toBits(
        const std::string& mnemonic);
    [[nodiscard]] static std::vector<std::uint8_t> toSeed(
        const std::string& mnemonic, const std::string& passphrase = "",
        std::function<void(std::uint32_t, std::uint32_t)> progressCallback
        = nullptr);
    [[nodiscard]] static int findWord(const std::string& word);
    [[nodiscard]] static std::string completeWord(const std::string& prefix);
    [[nodiscard]] static std::string getWord(int index);
    [[nodiscard]] static std::uint32_t getWordCompletionMask(
        const std::string& prefix);
};

}
#endif // BASEMNEMONIC_H
