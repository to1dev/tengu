// SPDX-License-Identifier: AGPL-3.0-or-later
/*
 * Copyright (C) 2025 to1dev <https://arc20.me/to1dev>
 *
 * A "product-level" Bincode library for C++.
 *
 * Key features:
 * 1) Little-endian integer serialization (bool, i8, u8, i16, u16, i32, u32,
 * i64, u64, i128, u128). 2) ShortVec / Varint (a.k.a. "compact_u64") for array
 * lengths or signature counts, etc. 3) Standard fixed-length writes for
 * integers (e.g. write_u64 => 8 bytes, LE). 4) Methods to read/write strings,
 * variable-size byte arrays with either "u64 prefix" or "compact prefix". 5)
 * Fixed-size byte arrays with no prefix. 6) Bound checks with exceptions
 * (std::runtime_error) on errors.
 *
 * If your compiler doesn't support (unsigned) __int128, please remove/replace
 * i128/u128 methods. This library is flexible but be consistent: if you wrote
 * something with compact_u64, read it with read_compact_u64, etc.
 *
 * For multi-thread usage, either create separate instances or add your own
 * locking.
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

namespace bincode {

// =============================================================
// BincodeWriter
// =============================================================

/**
 * @brief BincodeWriter: Write data in binary form into an internal buffer.
 *
 * - All multi-byte integers are written in little-endian.
 * - Provides both "compact" (shortvec) and normal fixed-size ways to encode
 * integer/length fields.
 * - Once done, get_buffer() to retrieve the final serialized bytes.
 */
class BincodeWriter {
public:
    //
    // === Basic bool / int (fixed size) ===
    //

    inline void write_bool(bool value)
    {
        buffer.push_back(value ? 1 : 0);
    }

    inline void write_i8(int8_t value)
    {
        buffer.push_back(static_cast<uint8_t>(value));
    }

    inline void write_u8(uint8_t value)
    {
        buffer.push_back(value);
    }

    inline void write_i16(int16_t value)
    {
        write_little_endian<uint16_t>(static_cast<uint16_t>(value));
    }

    inline void write_u16(uint16_t value)
    {
        write_little_endian<uint16_t>(value);
    }

    inline void write_i32(int32_t value)
    {
        write_little_endian<uint32_t>(static_cast<uint32_t>(value));
    }

    inline void write_u32(uint32_t value)
    {
        write_little_endian<uint32_t>(value);
    }

    inline void write_i64(int64_t value)
    {
        write_little_endian<uint64_t>(static_cast<uint64_t>(value));
    }

    inline void write_u64(uint64_t value)
    {
        write_little_endian<uint64_t>(value);
    }

    /**
     * @brief write_i128: writes 16 bytes in 2's complement little-endian form.
     * @note requires compiler support for __int128
     */
    inline void write_i128(__int128 value)
    {
        write_128bit_le<__int128>(value);
    }

    /**
     * @brief write_u128: writes 16 bytes in little-endian form
     * @note requires compiler support for unsigned __int128
     */
    inline void write_u128(unsigned __int128 value)
    {
        write_128bit_le<unsigned __int128>(value);
    }

    //
    // === ShortVec / Varint for lengths (compact) ===
    //
    /**
     * @brief write_compact_u64: writes a varint-style integer, using 7 bits per
     * byte
     */
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

    //
    // === Strings & bytes with normal (u64) length prefix or compact prefix ===
    //

    /**
     * @brief write_string_u64: writes a string with an 8-byte u64 length
     * prefix, then data
     */
    inline void write_string_u64(const std::string& str)
    {
        uint64_t len = static_cast<uint64_t>(str.size());
        write_u64(len);
        buffer.insert(buffer.end(),
            reinterpret_cast<const uint8_t*>(str.data()),
            reinterpret_cast<const uint8_t*>(str.data() + len));
    }

    /**
     * @brief write_string_compact: writes a string with a shortvec (compact)
     * length prefix
     */
    inline void write_string_compact(const std::string& str)
    {
        uint64_t len = static_cast<uint64_t>(str.size());
        write_compact_u64(len);
        buffer.insert(buffer.end(),
            reinterpret_cast<const uint8_t*>(str.data()),
            reinterpret_cast<const uint8_t*>(str.data() + len));
    }

    inline void write_bytes_u64(const std::vector<uint8_t>& vec)
    {
        uint64_t len = vec.size();
        write_u64(len);
        buffer.insert(buffer.end(), vec.begin(), vec.end());
    }

    inline void write_bytes_compact(const std::vector<uint8_t>& vec)
    {
        uint64_t len = vec.size();
        write_compact_u64(len);
        buffer.insert(buffer.end(), vec.begin(), vec.end());
    }

    //
    // === Fixed-size arrays and raw bytes (no length prefix) ===
    //

    /**
     * @brief write_fixed_bytes: writes a fixed-size array with no prefix
     * @example for a 32-byte pubkey
     */
    template <size_t N>
    inline void write_fixed_bytes(const std::array<uint8_t, N>& arr)
    {
        buffer.insert(buffer.end(), arr.begin(), arr.end());
    }

    /**
     * @brief write_raw_bytes: writes the given data with no prefix
     * @param data pointer
     * @param length byte length
     */
    inline void write_raw_bytes(const uint8_t* data, size_t length)
    {
        buffer.insert(buffer.end(), data, data + length);
    }

    /**
     * @brief get_buffer: get final serialized data
     */
    inline const std::vector<uint8_t>& get_buffer() const
    {
        return buffer;
    }

private:
    std::vector<uint8_t> buffer;

    /**
     * @brief write_little_endian: writes an unsigned integral type T in
     * little-endian
     */
    template <typename T>
    inline void write_little_endian(T value)
    {
        static_assert(
            std::is_unsigned<T>::value, "must be unsigned integral type");
        for (int i = 0; i < (int)sizeof(T); i++) {
            buffer.push_back(static_cast<uint8_t>(value & 0xFF));
            value >>= 8;
        }
    }

    /**
     * @brief write_128bit_le: for __int128 or unsigned __int128
     */
    template <typename T128>
    inline void write_128bit_le(T128 v)
    {
        for (int i = 0; i < 16; i++) {
            buffer.push_back(static_cast<uint8_t>(v & 0xFF));
            v >>= 8;
        }
    }
};

// =============================================================
// BincodeReader
// =============================================================

/**
 * @brief BincodeReader: read data from a binary buffer produced by
 * BincodeWriter (or equivalent).
 *
 * - Must use matching read_xxx for the writer's write_xxx.
 * - All multi-byte integers are little-endian.
 * - If out of range or length is invalid, throws std::runtime_error.
 */
class BincodeReader {
public:
    explicit BincodeReader(const std::vector<uint8_t>& data)
        : buffer(data)
        , pos(0)
    {
    }

    //
    // === Basic bool / int (fixed size) ===
    //

    inline bool read_bool()
    {
        checkSpace(1);
        uint8_t b = buffer[pos++];
        return (b != 0);
    }

    inline int8_t read_i8()
    {
        checkSpace(1);
        return static_cast<int8_t>(buffer[pos++]);
    }

    inline uint8_t read_u8()
    {
        checkSpace(1);
        return buffer[pos++];
    }

    inline int16_t read_i16()
    {
        // read as u16, reinterpret as i16
        uint16_t raw = read_little_endian<uint16_t>();
        return static_cast<int16_t>(raw);
    }

    inline uint16_t read_u16()
    {
        return read_little_endian<uint16_t>();
    }

    inline int32_t read_i32()
    {
        uint32_t raw = read_little_endian<uint32_t>();
        return static_cast<int32_t>(raw);
    }

    inline uint32_t read_u32()
    {
        return read_little_endian<uint32_t>();
    }

    inline int64_t read_i64()
    {
        uint64_t raw = read_little_endian<uint64_t>();
        return static_cast<int64_t>(raw);
    }

    inline uint64_t read_u64()
    {
        return read_little_endian<uint64_t>();
    }

    inline __int128 read_i128()
    {
        // read 16 bytes little-endian => do sign extension for negative
        unsigned __int128 uval = read_128bit_le();
        return sign_extend_i128(uval);
    }

    inline unsigned __int128 read_u128()
    {
        // read 16 bytes little-endian => interpret as unsigned
        return read_128bit_le();
    }

    //
    // === ShortVec / Varint for lengths (compact) ===
    //
    /**
     * @brief read_compact_u64: reads a varint-style integer from the buffer
     */
    inline uint64_t read_compact_u64()
    {
        uint64_t result = 0;
        int shift = 0;
        while (true) {
            checkSpace(1);
            uint8_t byte = buffer[pos++];
            // take 7 bits
            result |= static_cast<uint64_t>(byte & 0x7F) << shift;
            if ((byte & 0x80) == 0) {
                break; // last byte
            }
            shift += 7;
            if (shift > 63) {
                throw std::runtime_error(
                    "read_compact_u64: varint too big or corrupt");
            }
        }
        return result;
    }

    //
    // === Strings & bytes with normal (u64) length prefix or compact prefix ===
    //

    inline std::string read_string_u64()
    {
        uint64_t len = read_u64();
        checkSpace(len);
        // read 'len' bytes => string
        std::string s(reinterpret_cast<const char*>(&buffer[pos]), (size_t)len);
        pos += len;
        return s;
    }

    inline std::string read_string_compact()
    {
        uint64_t len = read_compact_u64();
        checkSpace(len);
        std::string s(reinterpret_cast<const char*>(&buffer[pos]), (size_t)len);
        pos += len;
        return s;
    }

    inline std::vector<uint8_t> read_bytes_u64()
    {
        uint64_t len = read_u64();
        checkSpace(len);
        std::vector<uint8_t> vec(
            buffer.begin() + pos, buffer.begin() + pos + (size_t)len);
        pos += len;
        return vec;
    }

    inline std::vector<uint8_t> read_bytes_compact()
    {
        uint64_t len = read_compact_u64();
        checkSpace(len);
        std::vector<uint8_t> vec(
            buffer.begin() + pos, buffer.begin() + pos + (size_t)len);
        pos += len;
        return vec;
    }

    //
    // === Fixed-size arrays and raw bytes (no length prefix) ===
    //

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
        std::vector<uint8_t> vec(
            buffer.begin() + pos, buffer.begin() + pos + length);
        pos += length;
        return vec;
    }

    //
    // === Position & check ===
    //

    inline bool isEOF() const
    {
        return pos >= buffer.size();
    }

    inline size_t remaining() const
    {
        return buffer.size() - pos;
    }

private:
    const std::vector<uint8_t>& buffer;
    size_t pos;

    /**
     * @brief checkSpace: throw if we do not have 'need' more bytes to read
     */
    inline void checkSpace(size_t need) const
    {
        if (pos + need > buffer.size()) {
            throw std::runtime_error(
                "BincodeReader: out of range or truncated data");
        }
    }

    /**
     * @brief read_little_endian: reads sizeof(T) bytes and interprets as a
     * little-endian unsigned T
     */
    template <typename T>
    inline T read_little_endian()
    {
        static_assert(
            std::is_unsigned<T>::value, "must be an unsigned integral type");
        checkSpace(sizeof(T));
        T val = 0;
        for (int i = 0; i < (int)sizeof(T); i++) {
            val |= static_cast<T>(buffer[pos++]) << (8 * i);
        }
        return val;
    }

    /**
     * @brief read_128bit_le: reads 16 bytes, returns as unsigned __int128
     */
    inline unsigned __int128 read_128bit_le()
    {
        checkSpace(16);
        unsigned __int128 uval = 0;
        for (int i = 0; i < 16; i++) {
            uval |= static_cast<unsigned __int128>(buffer[pos++]) << (8 * i);
        }
        return uval;
    }

    /**
     * @brief sign_extend_i128: interpret the top bit of a 128-bit value as sign
     * bit if i128
     */
    static inline __int128 sign_extend_i128(unsigned __int128 uval)
    {
        // check the 127th bit
        constexpr unsigned __int128 signBit
            = (static_cast<unsigned __int128>(1) << 127);
        bool negative = ((uval & signBit) != 0);
        if (!negative) {
            // positive
            return static_cast<__int128>(uval);
        } else {
            // 2's complement => i128
            unsigned __int128 complement = (~uval) + 1;
            __int128 negVal = -static_cast<__int128>(complement);
            return negVal;
        }
    }
};
}

/*
struct MyStruct {
    int32_t x;
    std::string name;

    void serialize(BincodeWriter& w) const {
        w.write_i32(x);
        w.write_string_compact(name);
    }

    void deserialize(BincodeReader& r) {
        x = r.read_i32();
        name = r.read_string_compact();
    }
};
*/

/*
#include <iostream>
#include "bincode.hpp"

int main() {
    using namespace bincode;

    BincodeWriter writer;

    writer.write_bool(true);
    writer.write_i32(-12345);
    writer.write_u64(9999999999ULL);

    writer.write_string_compact("hello bincode");

    std::vector<uint8_t> data{0xAB, 0xCD, 0xEF};
    writer.write_bytes_u64(data);

    const auto& buf = writer.get_buffer();
    std::cout << "Serialized length = " << buf.size() << std::endl;

    BincodeReader reader(buf);

    bool bVal = reader.read_bool();
    int32_t iVal = reader.read_i32();
    uint64_t uVal = reader.read_u64();
    std::string strVal = reader.read_string_compact();
    std::vector<uint8_t> arrVal = reader.read_bytes_u64();

    std::cout << "bVal=" << bVal
              << ", iVal=" << iVal
              << ", uVal=" << uVal
              << ", strVal='" << strVal << "'"
              << ", arrVal.size=" << arrVal.size() << std::endl;

    return 0;
}
*/
