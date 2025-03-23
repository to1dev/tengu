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
#include <cstring> // for std::memcpy
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

struct PublicKey {
    static constexpr std::size_t SIZE = 32;
    std::array<std::uint8_t, SIZE> data {};
};

struct U128 {
    std::uint64_t lo;
    std::uint64_t hi;
};

namespace borsh {

/**
 * @brief 从 data[pos] 开始读取 len 字节到 dst。
 *        若越界，则抛出 std::runtime_error。
 *
 * @param data 二进制数据缓冲
 * @param pos  读取起始偏移量，会自动递增 len
 * @param dst  目标内存地址
 * @param len  要读取的字节数
 */
inline void read_bytes(const std::vector<std::uint8_t>& data, size_t& pos,
    void* dst, std::size_t len)
{
    if (pos + len > data.size()) {
        throw std::runtime_error("borsh::read_bytes: out of range");
    }
    std::memcpy(dst, data.data() + pos, len);
    pos += len;
}

/**
 * @brief 反序列化的主调度结构体（主模板）。
 *        对于不支持的类型，会在编译期触发静态断言错误。
 */
template <typename T, typename Enable = void> struct Deserializer {
    static T deserialize(
        const std::vector<std::uint8_t>& /*data*/, size_t& /*pos*/)
    {
        static_assert(sizeof(T) == 0,
            "No Deserializer<T> specialization or SFINAE-enabled overload for "
            "this type T.");
        return {}; // 只为保证函数签名完整，不会真正执行
    }
};

/**
 * @brief 针对布尔类型的偏特化
 *        Borsh 写法通常是 1 字节表示 true/false
 */
template <> struct Deserializer<bool> {
    static bool deserialize(const std::vector<std::uint8_t>& data, size_t& pos)
    {
        std::uint8_t byteVal = 0;
        read_bytes(data, pos, &byteVal, sizeof(byteVal));
        return (byteVal != 0);
    }
};

/**
 * @brief 其他整型类型的通用偏特化（以 std::is_integral 来判断）
 *        你可以进一步拆分为 32位/64位或无符号/有符号做不同处理。
 */
template <typename T>
struct Deserializer<T, std::enable_if_t<std::is_integral_v<T>>> {
    static T deserialize(const std::vector<std::uint8_t>& data, size_t& pos)
    {
        T val {};
        // 假设 Borsh 以小端序直接写入，如需大端序可额外做字节翻转
        read_bytes(data, pos, &val, sizeof(T));
        // 如果需要大小端转换，可在这做翻转
        return val;
    }
};

/**
 * @brief 针对 std::string 的偏特化
 *        Borsh 通常先写一个 u32 表示字符串长度，然后写该长度的字节。
 */
template <> struct Deserializer<std::string> {
    static std::string deserialize(
        const std::vector<std::uint8_t>& data, size_t& pos)
    {
        // 先读取一个 32 位整型代表字符串长度
        const auto length = Deserializer<std::uint32_t>::deserialize(data, pos);

        if (pos + length > data.size()) {
            throw std::runtime_error(
                "borsh::Deserializer<std::string>: out of range");
        }
        std::string result(
            reinterpret_cast<const char*>(data.data() + pos), length);
        pos += length;
        return result;
    }
};

/**
 * @brief 针对 std::optional<T> 的偏特化
 *        Borsh 通常先写一个 bool 表示是否有值，然后若有值则写真正的 T。
 */
template <typename T> struct Deserializer<std::optional<T>> {
    static std::optional<T> deserialize(
        const std::vector<std::uint8_t>& data, size_t& pos)
    {
        // 先读一个 bool 表示是否有值
        bool hasValue = Deserializer<bool>::deserialize(data, pos);
        if (!hasValue) {
            return std::nullopt;
        }
        // 再读实际的 T
        T value = Deserializer<T>::deserialize(data, pos);
        return std::make_optional(std::move(value));
    }
};

/**
 * @brief 针对 std::vector<T> 的偏特化
 *        Borsh 通常先写一个 u32 表示元素个数，然后依次写每个元素。
 */
template <typename T> struct Deserializer<std::vector<T>> {
    static std::vector<T> deserialize(
        const std::vector<std::uint8_t>& data, size_t& pos)
    {
        // 先读取一个 u32 代表元素个数
        const auto count = Deserializer<std::uint32_t>::deserialize(data, pos);
        std::vector<T> vec;
        vec.reserve(count);

        for (std::uint32_t i = 0; i < count; ++i) {
            T elem = Deserializer<T>::deserialize(data, pos);
            vec.push_back(std::move(elem));
        }
        return vec;
    }
};

template <> struct Deserializer<PublicKey> {
    static PublicKey deserialize(
        const std::vector<std::uint8_t>& data, size_t& pos)
    {
        PublicKey pk {};
        read_bytes(data, pos, pk.data.data(), PublicKey::SIZE);
        return pk;
    }
};

// ----------------- 在此添加 U128 的特化 -----------------
// 例如假设 Borsh 用小端序存储 128位: 先低64再高64
template <> struct Deserializer<U128> {
    static U128 deserialize(const std::vector<std::uint8_t>& data, size_t& pos)
    {
        U128 val {};
        // 先读 low 64
        val.lo = Deserializer<std::uint64_t>::deserialize(data, pos);
        // 再读 high 64
        val.hi = Deserializer<std::uint64_t>::deserialize(data, pos);
        return val;
    }
};

/**
 * @brief 外部调用接口：通过 borsh::deserialize<T>(data, pos) 反序列化目标类型 T
 *        不同类型的具体逻辑由 Deserializer<T>::deserialize(...) 负责。
 *
 * @tparam T    目标类型
 * @param data  源字节数据
 * @param pos   当前偏移量（调用后会移动）
 * @return      反序列化得到的 T 实例
 */
template <typename T>
inline T deserialize(const std::vector<std::uint8_t>& data, size_t& pos)
{
    return Deserializer<T>::deserialize(data, pos);
}

} // namespace borsh
