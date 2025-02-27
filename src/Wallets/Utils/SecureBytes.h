#ifndef SECUREBYTES_H
#define SECUREBYTES_H

#include <cstdint>
#include <vector>

#include <sodium.h>

namespace Daitengu::Wallets {

class SecureBytes {
public:
    SecureBytes() = default;
    explicit SecureBytes(std::size_t size);
    explicit SecureBytes(const std::vector<std::uint8_t>& data);

    ~SecureBytes();

    SecureBytes(const SecureBytes&) = delete;
    SecureBytes& operator=(const SecureBytes&) = delete;

    SecureBytes(SecureBytes&& other) noexcept;
    SecureBytes& operator=(SecureBytes&& other) noexcept;

    void clear();

    const std::vector<std::uint8_t>& vec() const
    {
        return bytes_;
    }

    std::vector<std::uint8_t>& vec()
    {
        return bytes_;
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
