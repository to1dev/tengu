// SPDX-License-Identifier: AGPL-3.0-or-later
/*
 * Copyright (C) 2025 to1dev <https://arc20.me/to1dev>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace bincode {

class BincodeWriter {
public:
    inline void write_u8(uint8_t value)
    {
        buffer.push_back(value);
    }

    inline void write_u16(uint16_t value)
    {
        for (int i = 0; i < 2; i++) {
            buffer.push_back(static_cast<uint8_t>(value & 0xFF));
            value >>= 8;
        }
    }

    inline void write_u32(uint32_t value)
    {
        for (int i = 0; i < 4; i++) {
            buffer.push_back(static_cast<uint8_t>(value & 0xFF));
            value >>= 8;
        }
    }

    inline void write_u64(uint64_t value)
    {
        for (int i = 0; i < 8; i++) {
            buffer.push_back(static_cast<uint8_t>(value & 0xFF));
            value >>= 8;
        }
    }

    inline void write_u128(unsigned __int128 value)
    {
        for (int i = 0; i < 16; i++) {
            buffer.push_back(static_cast<uint8_t>(value & 0xFF));
            value >>= 8;
        }
    }

    inline void write_compact_u64(uint64_t value)
    {
        while (true) {
            uint8_t byte = static_cast<uint8_t>(value & 0x7F);
            value >>= 7;
            if (value != 0) {
                byte |= 0x80;
            }
            buffer.push_back(byte);
            if ((byte & 0x80) == 0) {
                break;
            }
        }
    }

    template <size_t N>
    inline void write_fixed_bytes(const std::array<uint8_t, N>& arr)
    {
        buffer.insert(buffer.end(), arr.begin(), arr.end());
    }

    inline void write_raw_bytes(const uint8_t* data, size_t length)
    {
        buffer.insert(buffer.end(), data, data + length);
    }

    inline const std::vector<uint8_t>& get_buffer() const
    {
        return buffer;
    }

private:
    std::vector<uint8_t> buffer;
};

class BincodeReader {
public:
    BincodeReader(const std::vector<uint8_t>& data)
        : buffer(data)
        , pos(0)
    {
    }

    inline uint8_t read_u8()
    {
        checkSpace(1);
        return buffer[pos++];
    }

    inline uint16_t read_u16()
    {
        checkSpace(2);
        uint16_t result = 0;
        for (int i = 0; i < 2; i++) {
            result |= (static_cast<uint16_t>(buffer[pos++]) << (8 * i));
        }
        return result;
    }

    inline uint32_t read_u32()
    {
        checkSpace(4);
        uint32_t result = 0;
        for (int i = 0; i < 4; i++) {
            result |= (static_cast<uint32_t>(buffer[pos++]) << (8 * i));
        }
        return result;
    }

    inline uint64_t read_u64()
    {
        checkSpace(8);
        uint64_t result = 0;
        for (int i = 0; i < 8; i++) {
            result |= (static_cast<uint64_t>(buffer[pos++]) << (8 * i));
        }
        return result;
    }

    inline unsigned __int128 read_u128()
    {
        checkSpace(16);
        unsigned __int128 result = 0;
        for (int i = 0; i < 16; i++) {
            result
                |= (static_cast<unsigned __int128>(buffer[pos++]) << (8 * i));
        }
        return result;
    }

    inline uint64_t read_compact_u64()
    {
        uint64_t result = 0;
        int shift = 0;
        while (true) {
            checkSpace(1);
            uint8_t byte = buffer[pos++];
            result |= static_cast<uint64_t>(byte & 0x7F) << shift;
            if ((byte & 0x80) == 0) {
                break;
            }
            shift += 7;
            if (shift > 63) {
                throw std::runtime_error("ShortVec: integer overflow");
            }
        }
        return result;
    }

    template <size_t N>
    inline std::array<uint8_t, N> read_fixed_bytes()
    {
        checkSpace(N);
        std::array<uint8_t, N> arr;
        std::memcpy(arr.data(), &buffer[pos], N);
        pos += N;
        return arr;
    }

    inline std::vector<uint8_t> read_raw_bytes(size_t length)
    {
        checkSpace(length);
        std::vector<uint8_t> result(
            buffer.begin() + pos, buffer.begin() + pos + length);
        pos += length;
        return result;
    }

    inline bool isEOF() const
    {
        return pos >= buffer.size();
    }

    inline size_t remaining() const
    {
        return buffer.size() - pos;
    }

private:
    inline void checkSpace(size_t need) const
    {
        if (pos + need > buffer.size()) {
            throw std::runtime_error("BincodeReader: out of range");
        }
    }

private:
    const std::vector<uint8_t>& buffer;
    size_t pos;
};
}
