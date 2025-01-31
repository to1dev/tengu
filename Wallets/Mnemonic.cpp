#include "Mnemonic.h"

namespace Daitengu::Wallets {

Mnemonic::Mnemonic()
{
}

std::string Mnemonic::generate(int strength)
{
    if (strength != 128 && strength != 160 && strength != 192 && strength != 224
        && strength != 256) {
        throw std::invalid_argument(
            "Invalid strength value. Must be 128, 160, 192, 224, or 256.");
    }

    const char* result = mnemonic_generate(strength);
    if (!result) {
        throw std::runtime_error("Failed to generate mnemonic.");
    }

    return std::string(result);
}

std::string Mnemonic::fromData(const std::vector<uint8_t>& data)
{
    const char* result = mnemonic_from_data(data.data(), data.size());

    if (!result) {
        throw std::runtime_error("Failed to generate mnemonic from data.");
    }

    mnemonic_clear();

    return std::string(result);
}

void Mnemonic::clear()
{
    mnemonic_clear();
}

bool Mnemonic::check(const std::string& mnemonic)
{
    return mnemonic_check(mnemonic.c_str()) == 1;
}

std::vector<uint8_t> Mnemonic::toBits(const std::string& mnemonic)
{
    std::vector<uint8_t> bits(512 / 8, 0);

    if (mnemonic_to_bits(mnemonic.c_str(), bits.data()) == 0) {
        throw std::runtime_error("Failed to convert mnemonic to bits.");
    }

    return bits;
}

std::vector<uint8_t> Mnemonic::toSeed(const std::string& mnemonic,
    const std::string& passphrase,
    std::function<void(uint32_t, uint32_t)> progressCallback)
{
    std::vector<uint8_t> seed(512 / 8, 0);

    static std::function<void(uint32_t, uint32_t)> callback;
    callback = progressCallback;

    auto wrapper = [](uint32_t current, uint32_t total) {
        if (callback) {
            callback(current, total);
        }
    };

    mnemonic_to_seed(
        mnemonic.c_str(), passphrase.c_str(), seed.data(), wrapper);

    return seed;
}

int Mnemonic::findWord(const std::string& word)
{
    return mnemonic_find_word(word.c_str());
}

std::string Mnemonic::completeWord(const std::string& prefix)
{
    const char* result = mnemonic_complete_word(prefix.c_str(), prefix.size());

    if (!result) {
        throw std::runtime_error("Failed to complete word.");
    }

    return std::string(result);
}

std::string Mnemonic::getWord(int index)
{
    const char* result = mnemonic_get_word(index);

    if (!result) {
        throw std::runtime_error("Invalid word index.");
    }

    return std::string(result);
}

uint32_t Mnemonic::getWordCompletionMask(const std::string& prefix)
{
    return mnemonic_word_completion_mask(prefix.c_str(), prefix.size());
}

}
