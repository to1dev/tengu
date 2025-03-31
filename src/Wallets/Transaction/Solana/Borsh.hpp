// SPDX-License-Identifier: AGPL-3.0-or-later
/*
 * Copyright (C) 2025 to1dev <https://arc20.me/to1dev>
 *
 * Borsh serialization/deserialization (a more complete, "production-level"
 * implementation)
 *
 * Features:
 * 1. All integers are written/read in little-endian order (includes i8, u8,
 * i16, u16, i32, u32, i64, u64, i128, u128)
 * 2. For variable-length sequences (like byte arrays, strings), a 4-byte (u32)
 * length prefix is used, followed by raw bytes
 * 3. Fixed-length data (e.g., 32-byte public key) does not require a length
 * prefix
 * 4. Throws std::runtime_error on buffer overrun or insufficient data
 * 5. 128-bit unsigned integers are only supported on compilers with __int128
 * support, otherwise use a 16-byte array
 *
 * Borsh specification: https://borsh.io
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
#include <string>
#include <type_traits>
#include <vector>

namespace borsh {

/**
 * @brief BorshWriter: Serializes data to memory buffer using Borsh format.
 *
 * Borsh rules:
 * - Integers are little-endian
 * - Sequences (e.g., string/vec<u8>) are prefixed with a 4-byte (u32) length,
 * followed by raw bytes
 * - i128/u128 are written as 16 bytes in little-endian; i128 is signed (2's
 * complement), u128 is unsigned
 */
class BorshWriter {
public:
    //
    //========== Write Boolean / Integer ==========
    //

    /**
     * @brief Write a bool (1 byte), 0x00 = false, 0x01 = true
     */
    inline void write_bool(bool value)
    {
        buffer.push_back(value ? 1 : 0);
    }

    /**
     * @brief Write 8-bit signed integer (i8)
     */
    inline void write_i8(int8_t value)
    {
        buffer.push_back(static_cast<uint8_t>(value));
    }

    /**
     * @brief Write 8-bit unsigned integer (u8)
     */
    inline void write_u8(uint8_t value)
    {
        buffer.push_back(value);
    }

    /**
     * @brief Write 16-bit signed integer (i16), little-endian
     */
    inline void write_i16(int16_t value)
    {
        writeLittleEndian(value);
    }

    /**
     * @brief Write 16-bit unsigned integer (u16), little-endian
     */
    inline void write_u16(uint16_t value)
    {
        writeLittleEndian(value);
    }

    /**
     * @brief Write 32-bit signed integer (i32), little-endian
     */
    inline void write_i32(int32_t value)
    {
        writeLittleEndian(value);
    }

    /**
     * @brief Write 32-bit unsigned integer (u32), little-endian
     */
    inline void write_u32(uint32_t value)
    {
        writeLittleEndian(value);
    }

    /**
     * @brief Write 64-bit signed integer (i64), little-endian
     */
    inline void write_i64(int64_t value)
    {
        writeLittleEndian(value);
    }

    /**
     * @brief Write 64-bit unsigned integer (u64), little-endian
     */
    inline void write_u64(uint64_t value)
    {
        writeLittleEndian(value);
    }

    /**
     * @brief Write 128-bit signed integer (i128), little-endian (2's
     * complement)
     *
     * Requires compiler support for (unsigned) __int128.
     * Writes 16 bytes in little-endian order representing 2's complement of
     * `int128`.
     */
    inline void write_i128(__int128 value)
    {
        write128bitLittleEndian(value);
    }

    /**
     * @brief Write 128-bit unsigned integer (u128), little-endian
     *
     * Same as above, requires __int128 support
     */
    inline void write_u128(unsigned __int128 value)
    {
        write128bitLittleEndian(value);
    }

    //
    //========== Write Strings / Binary Arrays ==========
    //

    /**
     * @brief Write std::string (Borsh: 4-byte (u32) length prefix, followed by
     * raw bytes)
     */
    inline void write_string(const std::string& str)
    {
        write_u32(static_cast<uint32_t>(str.size()));
        buffer.insert(buffer.end(),
            reinterpret_cast<const uint8_t*>(str.data()),
            reinterpret_cast<const uint8_t*>(str.data() + str.size()));
    }

    /**
     * @brief Write binary array (4-byte length prefix + raw bytes)
     */
    inline void write_bytes(const std::vector<uint8_t>& vec)
    {
        write_u32(static_cast<uint32_t>(vec.size()));
        buffer.insert(buffer.end(), vec.begin(), vec.end());
    }

    /**
     * @brief Write raw bytes with length prefix
     */
    inline void write_bytes(const uint8_t* data, size_t length)
    {
        if (length > 0xFFFFFFFFULL) {
            throw std::runtime_error("BorshWriter: length exceeds u32 max");
        }
        write_u32(static_cast<uint32_t>(length));
        buffer.insert(buffer.end(), data, data + length);
    }

    //
    //========== Write Fixed-Length / Raw Data ==========
    //

    /**
     * @brief Write fixed-length byte array (no length prefix),
     *        commonly used for 32-byte public keys, 64-byte signatures, etc.
     */
    template <size_t N>
    inline void write_fixed_bytes(const std::array<uint8_t, N>& arr)
    {
        buffer.insert(buffer.end(), arr.begin(), arr.end());
    }

    /**
     * @brief Write raw bytes without length prefix
     * @param data pointer to data
     * @param length number of bytes
     */
    inline void write_raw_bytes(const uint8_t* data, size_t length)
    {
        buffer.insert(buffer.end(), data, data + length);
    }

    /**
     * @brief Get the final serialized result
     */
    inline const std::vector<uint8_t>& get_buffer() const
    {
        return buffer;
    }

private:
    std::vector<uint8_t> buffer;

    /**
     * @brief Generic little-endian write function for integral types (2/4/8
     * bytes)
     */
    template <typename T>
    inline void writeLittleEndian(T value)
    {
        static_assert(std::is_integral<T>::value, "must be integral type");
        for (int i = 0; i < (int)sizeof(T); i++) {
            buffer.push_back(static_cast<uint8_t>(value & 0xFF));
            value >>= 8;
        }
    }

    /**
     * @brief Write 128-bit value in little-endian
     */
    template <typename T128>
    inline void write128bitLittleEndian(T128 value)
    {
        for (int i = 0; i < 16; i++) {
            buffer.push_back(static_cast<uint8_t>(value & 0xFF));
            value >>= 8;
        }
    }
};

/**
 * @brief BorshReader: Deserialize data from a Borsh-formatted byte buffer
 *
 * Requires a reference to a serialized byte buffer. No copying.
 * Use read_* methods to read each type. Will throw std::runtime_error on buffer
 * overrun or format error.
 */
class BorshReader {
public:
    explicit BorshReader(const std::vector<uint8_t>& data)
        : buffer(data)
        , pos(0)
    {
    }

    //
    //========== Read Boolean / Integer ==========
    //

    /**
     * @brief Read bool (1 byte), 0 => false, non-zero => true
     */
    inline bool read_bool()
    {
        checkSpace(1);
        uint8_t b = buffer[pos++];
        return (b != 0);
    }

    /**
     * @brief Read 8-bit signed integer (i8)
     */
    inline int8_t read_i8()
    {
        checkSpace(1);
        return static_cast<int8_t>(buffer[pos++]);
    }

    /**
     * @brief Read 8-bit unsigned integer (u8)
     */
    inline uint8_t read_u8()
    {
        checkSpace(1);
        return buffer[pos++];
    }

    /**
     * @brief Read 16-bit signed integer (i16), little-endian
     */
    inline int16_t read_i16()
    {
        return static_cast<int16_t>(readLittleEndian<uint16_t>());
    }

    /**
     * @brief Read 16-bit unsigned integer (u16), little-endian
     */
    inline uint16_t read_u16()
    {
        return readLittleEndian<uint16_t>();
    }

    /**
     * @brief Read 32-bit signed integer (i32), little-endian
     */
    inline int32_t read_i32()
    {
        return static_cast<int32_t>(readLittleEndian<uint32_t>());
    }

    /**
     * @brief Read 32-bit unsigned integer (u32), little-endian
     */
    inline uint32_t read_u32()
    {
        return readLittleEndian<uint32_t>();
    }

    /**
     * @brief Read 64-bit signed integer (i64), little-endian
     */
    inline int64_t read_i64()
    {
        return static_cast<int64_t>(readLittleEndian<uint64_t>());
    }

    /**
     * @brief Read 64-bit unsigned integer (u64), little-endian
     */
    inline uint64_t read_u64()
    {
        return readLittleEndian<uint64_t>();
    }

    /**
     * @brief Read 128-bit signed integer (i128), little-endian
     */
    inline __int128 read_i128()
    {
        return static_cast<__int128>(read128bitLittleEndian());
    }

    /**
     * @brief Read 128-bit unsigned integer (u128), little-endian
     */
    inline unsigned __int128 read_u128()
    {
        return static_cast<unsigned __int128>(read128bitLittleEndian());
    }

    //
    //========== Read Strings / Binary Arrays ==========
    //

    /**
     * @brief Read Borsh string (4-byte length prefix, then raw bytes)
     */
    inline std::string read_string()
    {
        uint32_t length = read_u32();
        checkSpace(length);
        std::string s(reinterpret_cast<const char*>(&buffer[pos]), length);
        pos += length;
        return s;
    }

    /**
     * @brief Read binary array (4-byte length prefix, then raw bytes)
     */
    inline std::vector<uint8_t> read_bytes()
    {
        uint32_t length = read_u32();
        checkSpace(length);
        std::vector<uint8_t> result(
            buffer.begin() + pos, buffer.begin() + pos + length);
        pos += length;
        return result;
    }

    //
    //========== Read Fixed-Length / Raw Data ==========
    //

    /**
     * @brief Read fixed-length byte array (no length prefix),
     *        used for 32-byte pubkey, 64-byte signature, etc.
     */
    template <size_t N>
    inline std::array<uint8_t, N> read_fixed_bytes()
    {
        checkSpace(N);
        std::array<uint8_t, N> arr;
        std::memcpy(arr.data(), &buffer[pos], N);
        pos += N;
        return arr;
    }

    /**
     * @brief Read raw bytes of given length (no length prefix)
     */
    inline std::vector<uint8_t> read_raw_bytes(size_t length)
    {
        checkSpace(length);
        std::vector<uint8_t> result(
            buffer.begin() + pos, buffer.begin() + pos + length);
        pos += length;
        return result;
    }

    /**
     * @brief Whether the end of buffer has been reached
     */
    inline bool isEOF() const
    {
        return pos >= buffer.size();
    }

    /**
     * @brief Get remaining bytes
     */
    inline size_t remaining() const
    {
        return buffer.size() - pos;
    }

private:
    const std::vector<uint8_t>& buffer;
    size_t pos;

    /**
     * @brief Ensure sufficient remaining bytes, otherwise throw exception
     */
    inline void checkSpace(size_t need) const
    {
        if (pos + need > buffer.size()) {
            throw std::runtime_error(
                "BorshReader: out of range or truncated data");
        }
    }

    /**
     * @brief Generic little-endian reader: reads sizeof(T) bytes and returns
     * unsigned value
     */
    template <typename T>
    inline T readLittleEndian()
    {
        static_assert(
            std::is_unsigned<T>::value, "must be unsigned integral type");
        checkSpace(sizeof(T));
        T value = 0;
        for (int i = 0; i < (int)sizeof(T); i++) {
            value |= static_cast<T>(buffer[pos++]) << (8 * i);
        }
        return value;
    }

    /**
     * @brief Read 16-byte little-endian data and convert to (signed) __int128
     *        TIPS: Borsh uses 2's complement for i128
     */
    inline __int128 read128bitLittleEndian()
    {
        checkSpace(16);
        unsigned __int128 uval = 0;
        for (int i = 0; i < 16; i++) {
            uval |= static_cast<unsigned __int128>(buffer[pos++]) << (8 * i);
        }
        return static_cast<__int128>(signExtend128(uval));
    }

    /**
     * @brief Sign-extend a 128-bit unsigned value to signed __int128 (2's
     * complement)
     */
    static inline __int128 signExtend128(unsigned __int128 val)
    {
        const unsigned __int128 signBit
            = (static_cast<unsigned __int128>(1) << 127);
        bool negative = ((val & signBit) != 0);
        if (!negative) {
            return static_cast<__int128>(val);
        } else {
            unsigned __int128 complement = (~val) + 1;
            __int128 s = -static_cast<__int128>(complement);
            return s;
        }
    }
};

} // namespace borsh

/*
struct MyData {
    int32_t x;
    std::string name;

    void borshSerialize(borsh::BorshWriter& writer) const {
        writer.write_i32(x);
        writer.write_string(name);
    }

    void borshDeserialize(borsh::BorshReader& reader) {
        x = reader.read_i32();
        name = reader.read_string();
    }
};
*/

/*
#include <iostream>
#include "borsh.hpp"

int main() {
    borsh::BorshWriter writer;

    writer.write_bool(true);
    writer.write_i32(-123456);
    writer.write_string("hello");
    writer.write_u64(9999999999ULL);

    std::vector<uint8_t> blob{ 0xAB, 0xCD, 0xEF };
    writer.write_bytes(blob);

    const auto& bytes = writer.get_buffer();
    std::cout << "Serialized borsh data length = " << bytes.size() << std::endl;

    borsh::BorshReader reader(bytes);

    bool bVal = reader.read_bool();
    int32_t iVal = reader.read_i32();
    std::string strVal = reader.read_string();
    uint64_t uVal = reader.read_u64();
    std::vector<uint8_t> arrVal = reader.read_bytes();

    std::cout << "bVal=" << (bVal ? "true" : "false")
              << ", iVal=" << iVal
              << ", strVal=" << strVal
              << ", uVal=" << uVal
              << ", arrVal size=" << arrVal.size() << std::endl;

    return 0;
}
*/
