#include "SecureBytes.h"

namespace Daitengu::Wallets {

SecureBytes::SecureBytes(const std::vector<uint8_t>& data)
    : bytes_(data)
{
}

SecureBytes::SecureBytes(std::size_t size)
    : bytes_(size, 0)
{
}

SecureBytes::~SecureBytes()
{
    clear();
}

SecureBytes::SecureBytes(SecureBytes&& other) noexcept
    : bytes_(std::move(other.bytes_))
{
    other.bytes_.clear();
}

SecureBytes& SecureBytes::operator=(SecureBytes&& other) noexcept
{
    if (this != &other) {
        clear();
        bytes_ = std::move(other.bytes_);
        other.bytes_.clear();
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
