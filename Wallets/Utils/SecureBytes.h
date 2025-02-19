#ifndef SECUREBYTES_H
#define SECUREBYTES_H

#include <cstdint>
#include <span>
#include <vector>

#include <sodium.h>

namespace Daitengu::Wallets {

class SecureBytes {
public:
    explicit SecureBytes(std::span<const std::uint8_t> data);
    explicit SecureBytes(std::size_t size);

    ~SecureBytes();

    SecureBytes(const SecureBytes&) = delete;
    SecureBytes& operator=(const SecureBytes&) = delete;

    SecureBytes(SecureBytes&& other) noexcept;
    SecureBytes& operator=(SecureBytes&& other) noexcept;

    void clear();

    std::span<const std::uint8_t> span() const
    {
        return std::span<const std::uint8_t>(bytes_);
    }

    std::span<std::uint8_t> span()
    {
        return std::span<std::uint8_t>(bytes_);
    }

    std::uint8_t* data()
    {
        return bytes_.data();
    }

    const std::uint8_t* data() const
    {
        return bytes_.data();
    }

    std::size_t size() const
    {
        return bytes_.size();
    }

private:
    std::vector<std::uint8_t> bytes_;
};

}
#endif // SECUREBYTES_H
