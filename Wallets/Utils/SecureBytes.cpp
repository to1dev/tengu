#include "SecureBytes.h"

namespace Daitengu::Wallets {

SecureBytes::SecureBytes(std::span<const std::uint8_t> data)
    : bytes_(data.begin(), data.end())
{
}

SecureBytes::SecureBytes(std::size_t size)
    : bytes_(size)
{
}

SecureBytes::~SecureBytes()
{
    clear();
}

SecureBytes::SecureBytes(SecureBytes&& other) noexcept
    : bytes_(std::move(other.bytes_))
{
}

SecureBytes& SecureBytes::operator=(SecureBytes&& other) noexcept
{
    if (this != &other) {
        clear();
        bytes_ = std::move(other.bytes_);
    }

    return *this;
}

void SecureBytes::clear()
{
    if (!bytes_.empty()) {
        sodium_memzero(bytes_.data(), bytes_.size());
        bytes_.clear();
    }
}

}
