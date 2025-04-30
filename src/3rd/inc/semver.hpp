//          _____                            _   _
//         / ____|                          | | (_)
//        | (___   ___ _ __ ___   __ _ _ __ | |_ _  ___
//         \___ \ / _ \ '_ ` _ \ / _` | '_ \| __| |/ __|
//         ____) |  __/ | | | | | (_| | | | | |_| | (__
//        |_____/ \___|_| |_| |_|__,_|_| |_|__,_|_|\___|
// __      __           _             _                _____
// \ \    / /          (_)           (_)              / ____|_     _
//  \ \  / /__ _ __ ___ _  ___  _ __  _ _ __   __ _  | |   _| |_ _| |_
//   \ \/ / _ \ '__/ __| |/ _ \| '_ \| | '_ \ / _` | | |  |_   _|_   _|
//    \  /  __/ |  \__ \ | (_) | | | | | | | | (_| | | |____|_|   |_|
//     \/ \___|_|  |___/_|\___/|_| |_|_|_| |_|__,_|  \_____|
// https://semver.org
// version 1.0.0
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2025 to1dev <https://arc20.me/to1dev>.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef SEMVER_HPP
#define SEMVER_HPP

#define SEMVER_VERSION_MAJOR 1
#define SEMVER_VERSION_MINOR 0
#define SEMVER_VERSION_PATCH 0

#include <charconv>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iosfwd>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#if defined(SEMVER_THROW)
#elif defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
#define SEMVER_THROW(msg) (throw std::invalid_argument { msg })
#else
#include <cassert>
#include <cstdlib>
#define SEMVER_THROW(msg) (assert(!msg), std::abort())
#endif

namespace semver {

// Enumeration for parsing errors
enum class parse_error {
    none,              // No error
    invalid_format,    // Invalid version format
    leading_zero,      // Leading zero in numeric field
    empty_identifier,  // Empty prerelease or build identifier
    invalid_character, // Invalid character in version string
    out_of_range       // Numeric value out of range
};

// Structure for parsing results, extending std::from_chars_result
struct from_chars_result : std::from_chars_result {
    parse_error error = parse_error::none;

    [[nodiscard]] constexpr operator bool() const noexcept
    {
        return ec == std::errc {} && error == parse_error::none;
    }
};

// Structure for formatting results, extending std::to_chars_result
struct to_chars_result : std::to_chars_result {
    [[nodiscard]] constexpr operator bool() const noexcept
    {
        return ec == std::errc {};
    }
};

// Maximum length of a version string: 5(major) + 1(.) + 5(minor) + 1(.) +
// 5(patch) + 1(-) + 32(prerelease) + 1(+) + 32(build)
inline constexpr auto max_version_string_length = std::size_t { 83 };

namespace detail {

    // Convert a character to lowercase
    constexpr char to_lower(char c) noexcept
    {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + ('a' - 'A')) : c;
    }

    // Check if a character is a digit
    constexpr bool is_digit(char c) noexcept
    {
        return c >= '0' && c <= '9';
    }

    // Check if a character is a letter
    constexpr bool is_letter(char c) noexcept
    {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    }

    // Check if a character is alphanumeric
    constexpr bool is_alphanumeric(char c) noexcept
    {
        return is_digit(c) || is_letter(c);
    }

    // Check if a character is a hyphen
    constexpr bool is_hyphen(char c) noexcept
    {
        return c == '-';
    }

    // Check if a character is a dot
    constexpr bool is_dot(char c) noexcept
    {
        return c == '.';
    }

    // Check if a character is a plus sign
    constexpr bool is_plus(char c) noexcept
    {
        return c == '+';
    }

    // Check if a character is a space
    constexpr bool is_space(char c) noexcept
    {
        return c == ' ';
    }

    // Check if a character is an operator (<, >, =)
    constexpr bool is_operator(char c) noexcept
    {
        return c == '<' || c == '>' || c == '=';
    }

    // Check if a character is a logical OR (|)
    constexpr bool is_logical_or(char c) noexcept
    {
        return c == '|';
    }

    // Check if a character is valid for an identifier (alphanumeric or hyphen)
    constexpr bool is_valid_identifier_char(char c) noexcept
    {
        return is_alphanumeric(c) || is_hyphen(c);
    }

    // Convert a digit character to its numeric value
    constexpr std::uint16_t to_digit(char c) noexcept
    {
        return static_cast<std::uint16_t>(c - '0');
    }

    // Check if a string is a valid numeric identifier (no leading zero unless
    // zero itself)
    constexpr bool is_numeric_identifier(std::string_view str) noexcept
    {
        if (str.empty() || str == "0")
            return false;
        if (str[0] == '0' && str.length() > 1)
            return false;
        return std::all_of(str.begin(), str.end(), is_digit);
    }

    // Calculate the number of digits in an unsigned integer
    constexpr std::uint8_t length(std::uint64_t x) noexcept
    {
        if (x == 0)
            return 1;
        std::uint8_t len = 0;
        while (x > 0) {
            x /= 10;
            ++len;
        }
        return len;
    }

    // Compare two strings case-insensitively
    constexpr bool equals(std::string_view a, std::string_view b) noexcept
    {
        if (a.length() != b.length())
            return false;
        for (std::size_t i = 0; i < a.length(); ++i) {
            if (to_lower(a[i]) != to_lower(b[i]))
                return false;
        }
        return true;
    }

    // Parse a numeric value from a string
    inline const char* from_chars(const char* first, const char* last,
        std::uint64_t& d, parse_error& err) noexcept
    {
        if (first == last || !is_digit(*first)) {
            err = parse_error::invalid_format;
            return nullptr;
        }
        std::uint64_t t = 0;
        std::size_t digits = 0;
        for (; first != last && is_digit(*first); ++first, ++digits) {
            t = t * 10 + to_digit(*first);
        }
        if (t > (std::numeric_limits<std::uint64_t>::max)()) {
            err = parse_error::out_of_range;
            return nullptr;
        }
        if (digits > 1 && t != 0 && first[-digits] == '0') {
            err = parse_error::leading_zero;
            return nullptr;
        }
        d = t;
        return first;
    }

    // Parse an identifier (alphanumeric or hyphen) from a string
    inline const char* from_chars_identifier(const char* first,
        const char* last, std::string& id, parse_error& err) noexcept
    {
        if (first == last || !is_alphanumeric(*first)) {
            err = parse_error::invalid_format;
            return nullptr;
        }
        std::string t;
        while (first != last && is_valid_identifier_char(*first)) {
            t += *first++;
        }
        if (t.empty()) {
            err = parse_error::empty_identifier;
            return nullptr;
        }
        id = t;
        return first;
    }

    // Format an unsigned integer to a string, optionally with a dot
    inline char* to_chars(char* str, std::uint64_t x, bool dot = true) noexcept
    {
        do {
            *(--str) = static_cast<char>('0' + (x % 10));
            x /= 10;
        } while (x != 0);
        if (dot)
            *(--str) = '.';
        return str;
    }

    // Check if a delimiter exists at the current position
    constexpr bool check_delimiter(
        const char* first, const char* last, char d) noexcept
    {
        return first != last && first != nullptr && *first == d;
    }

} // namespace detail

// Version structure representing a semantic version
struct version {
    std::uint64_t major = 0;             // Major version number
    std::uint64_t minor = 0;             // Minor version number
    std::uint64_t patch = 0;             // Patch version number
    std::vector<std::string> prerelease; // Prerelease identifiers
    std::vector<std::string> build;      // Build metadata

    // Construct a version with specified components
    constexpr version(std::uint64_t mj, std::uint64_t mn, std::uint64_t pt,
        std::vector<std::string> pr = {},
        std::vector<std::string> b = {}) noexcept
        : major { mj }
        , minor { mn }
        , patch { pt }
        , prerelease { std::move(pr) }
        , build { std::move(b) }
    {
    }

    // Construct a version from a string
    explicit version(std::string_view str)
    {
        from_string(str);
    }

    version() = default;
    version(const version&) = default;
    version(version&&) = default;
    ~version() = default;
    version& operator=(const version&) = default;
    version& operator=(version&&) = default;

    // Parse a version string into components
    from_chars_result from_chars(const char* first, const char* last) noexcept
    {
        if (first == nullptr || last == nullptr || (last - first) < 5) {
            return { first, std::errc::invalid_argument,
                parse_error::invalid_format };
        }

        parse_error err = parse_error::none;
        auto next = first;
        if (next = detail::from_chars(next, last, major, err); !next) {
            return { first, std::errc::invalid_argument, err };
        }
        if (!detail::check_delimiter(next, last, '.')) {
            return { first, std::errc::invalid_argument,
                parse_error::invalid_format };
        }
        if (next = detail::from_chars(++next, last, minor, err); !next) {
            return { first, std::errc::invalid_argument, err };
        }
        if (!detail::check_delimiter(next, last, '.')) {
            return { first, std::errc::invalid_argument,
                parse_error::invalid_format };
        }
        if (next = detail::from_chars(++next, last, patch, err); !next) {
            return { first, std::errc::invalid_argument, err };
        }

        if (next != last && detail::check_delimiter(next, last, '-')) {
            ++next;
            while (next != last && !detail::check_delimiter(next, last, '+')) {
                std::string id;
                if (next = detail::from_chars_identifier(next, last, id, err);
                    !next) {
                    return { first, std::errc::invalid_argument, err };
                }
                prerelease.push_back(id);
                if (next == last || detail::check_delimiter(next, last, '+'))
                    break;
                if (!detail::check_delimiter(next, last, '.')) {
                    return { first, std::errc::invalid_argument,
                        parse_error::invalid_format };
                }
                ++next;
            }
        }

        if (next != last && detail::check_delimiter(next, last, '+')) {
            ++next;
            while (next != last) {
                std::string id;
                if (next = detail::from_chars_identifier(next, last, id, err);
                    !next) {
                    return { first, std::errc::invalid_argument, err };
                }
                build.push_back(id);
                if (next == last)
                    break;
                if (!detail::check_delimiter(next, last, '.')) {
                    return { first, std::errc::invalid_argument,
                        parse_error::invalid_format };
                }
                ++next;
            }
        }

        if (next != last) {
            return { first, std::errc::invalid_argument,
                parse_error::invalid_format };
        }

        return { next, std::errc {}, parse_error::none };
    }

    // Format the version to a string
    to_chars_result to_chars(char* first, char* last) const noexcept
    {
        const auto length = string_length();
        if (first == nullptr || last == nullptr || (last - first) < length) {
            return { last, std::errc::value_too_large };
        }

        auto next = first + length;
        if (!build.empty()) {
            for (auto it = build.rbegin(); it != build.rend(); ++it) {
                for (auto c = it->rbegin(); c != it->rend(); ++c) {
                    *(--next) = *c;
                }
                if (it + 1 != build.rend())
                    *(--next) = '.';
            }
            *(--next) = '+';
        }
        if (!prerelease.empty()) {
            for (auto it = prerelease.rbegin(); it != prerelease.rend(); ++it) {
                for (auto c = it->rbegin(); c != it->rend(); ++c) {
                    *(--next) = *c;
                }
                if (it + 1 != prerelease.rend())
                    *(--next) = '.';
            }
            *(--next) = '-';
        }
        next = detail::to_chars(next, patch);
        next = detail::to_chars(next, minor);
        next = detail::to_chars(next, major, false);

        return { first + length, std::errc {} };
    }

    // Parse a version string without throwing exceptions
    bool from_string_noexcept(std::string_view str) noexcept
    {
        return from_chars(str.data(), str.data() + str.length());
    }

    // Parse a version string, throwing on error
    version& from_string(std::string_view str)
    {
        auto result = from_chars(str.data(), str.data() + str.length());
        if (!result) {
            std::string msg = std::format(
                "semver::version::from_string invalid version: {}", str);
            switch (result.error) {
            case parse_error::leading_zero:
                msg += " (leading zero)";
                break;
            case parse_error::empty_identifier:
                msg += " (empty identifier)";
                break;
            case parse_error::invalid_character:
                msg += " (invalid character)";
                break;
            case parse_error::out_of_range:
                msg += " (number out of range)";
                break;
            default:
                break;
            }
            SEMVER_THROW(msg);
        }
        return *this;
    }

    // Convert the version to a string
    std::string to_string() const
    {
        std::string str;
        str.resize(string_length());
        if (!to_chars(str.data(), str.data() + str.length())) {
            SEMVER_THROW("semver::version::to_string invalid version");
        }
        return str;
    }

    // Calculate the length of the version string
    std::size_t string_length() const noexcept
    {
        std::size_t len = detail::length(major) + detail::length(minor)
            + detail::length(patch) + 2;
        if (!prerelease.empty()) {
            len += 1;
            for (const auto& id : prerelease) {
                len += id.length();
            }
            len += prerelease.size() - 1;
        }
        if (!build.empty()) {
            len += 1;
            for (const auto& id : build) {
                len += id.length();
            }
            len += build.size() - 1;
        }
        return len;
    }

    // Compare two versions
    int compare(const version& other) const noexcept
    {
        if (major != other.major)
            return major < other.major ? -1 : 1;
        if (minor != other.minor)
            return minor < other.minor ? -1 : 1;
        if (patch != other.patch)
            return patch < other.patch ? -1 : 1;

        if (prerelease.empty() && !other.prerelease.empty())
            return 1;
        if (!prerelease.empty() && other.prerelease.empty())
            return -1;
        if (!prerelease.empty() && !other.prerelease.empty()) {
            std::size_t min_size
                = std::min(prerelease.size(), other.prerelease.size());
            for (std::size_t i = 0; i < min_size; ++i) {
                const auto& a = prerelease[i];
                const auto& b = other.prerelease[i];
                if (detail::is_numeric_identifier(a)
                    && detail::is_numeric_identifier(b)) {
                    std::uint64_t na = std::stoull(a);
                    std::uint64_t nb = std::stoull(b);
                    if (na != nb)
                        return na < nb ? -1 : 1;
                } else {
                    if (a != b)
                        return a < b ? -1 : 1;
                }
            }
            if (prerelease.size() != other.prerelease.size()) {
                return prerelease.size() < other.prerelease.size() ? -1 : 1;
            }
        }
        return 0;
    }
};

// Equality comparison
inline bool operator==(const version& lhs, const version& rhs) noexcept
{
    return lhs.compare(rhs) == 0;
}

// Inequality comparison
inline bool operator!=(const version& lhs, const version& rhs) noexcept
{
    return lhs.compare(rhs) != 0;
}

// Greater than comparison
inline bool operator>(const version& lhs, const version& rhs) noexcept
{
    return lhs.compare(rhs) > 0;
}

// Greater than or equal comparison
inline bool operator>=(const version& lhs, const version& rhs) noexcept
{
    return lhs.compare(rhs) >= 0;
}

// Less than comparison
inline bool operator<(const version& lhs, const version& rhs) noexcept
{
    return lhs.compare(rhs) < 0;
}

// Less than or equal comparison
inline bool operator<=(const version& lhs, const version& rhs) noexcept
{
    return lhs.compare(rhs) <= 0;
}

// Three-way comparison (C++20)
inline std::strong_ordering operator<=>(
    const version& lhs, const version& rhs) noexcept
{
    int cmp = lhs.compare(rhs);
    return cmp == 0 ? std::strong_ordering::equal
        : cmp > 0   ? std::strong_ordering::greater
                    : std::strong_ordering::less;
}

// User-defined literal for version strings
inline version operator""_version(const char* str, std::size_t length)
{
    return version { std::string_view { str, length } };
}

// Check if a version string is valid
inline bool valid(std::string_view str) noexcept
{
    return version {}.from_string_noexcept(str);
}

// Parse a version string into a version object
inline from_chars_result from_chars(
    const char* first, const char* last, version& v) noexcept
{
    return v.from_chars(first, last);
}

// Format a version object to a string
inline to_chars_result to_chars(
    char* first, char* last, const version& v) noexcept
{
    return v.to_chars(first, last);
}

// Parse a version string without throwing exceptions
inline std::optional<version> from_string_noexcept(
    std::string_view str) noexcept
{
    version v;
    if (v.from_string_noexcept(str)) {
        return v;
    }
    return std::nullopt;
}

// Parse a version string, throwing on error
inline version from_string(std::string_view str)
{
    return version { str };
}

// Convert a version to a string
inline std::string to_string(const version& v)
{
    return v.to_string();
}

// Stream output operator
template <typename Char, typename Traits>
inline std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>& os, const version& v)
{
    return os << v.to_string();
}

inline namespace comparators {

    // Options for version comparison
    enum struct comparators_option : std::uint8_t {
        exclude_prerelease, // Exclude prerelease identifiers in comparison
        include_prerelease  // Include prerelease identifiers in comparison
    };

    // Compare two versions with specified options
    inline int compare(const version& lhs, const version& rhs,
        comparators_option option
        = comparators_option::include_prerelease) noexcept
    {
        if (option == comparators_option::exclude_prerelease) {
            if (lhs.major != rhs.major)
                return lhs.major < rhs.major ? -1 : 1;
            if (lhs.minor != rhs.minor)
                return lhs.minor < rhs.minor ? -1 : 1;
            if (lhs.patch != rhs.patch)
                return lhs.patch < rhs.patch ? -1 : 1;
            return 0;
        }
        return lhs.compare(rhs);
    }

    // Check if two versions are equal
    inline bool equal_to(const version& lhs, const version& rhs,
        comparators_option option
        = comparators_option::include_prerelease) noexcept
    {
        return compare(lhs, rhs, option) == 0;
    }

    // Check if two versions are not equal
    inline bool not_equal_to(const version& lhs, const version& rhs,
        comparators_option option
        = comparators_option::include_prerelease) noexcept
    {
        return compare(lhs, rhs, option) != 0;
    }

    // Check if one version is greater than another
    inline bool greater(const version& lhs, const version& rhs,
        comparators_option option
        = comparators_option::include_prerelease) noexcept
    {
        return compare(lhs, rhs, option) > 0;
    }

    // Check if one version is greater than or equal to another
    inline bool greater_equal(const version& lhs, const version& rhs,
        comparators_option option
        = comparators_option::include_prerelease) noexcept
    {
        return compare(lhs, rhs, option) >= 0;
    }

    // Check if one version is less than another
    inline bool less(const version& lhs, const version& rhs,
        comparators_option option
        = comparators_option::include_prerelease) noexcept
    {
        return compare(lhs, rhs, option) < 0;
    }

    // Check if one version is less than or equal to another
    inline bool less_equal(const version& lhs, const version& rhs,
        comparators_option option
        = comparators_option::include_prerelease) noexcept
    {
        return compare(lhs, rhs, option) <= 0;
    }

} // namespace semver::comparators

namespace range {

    // Options for range satisfaction
    enum struct satisfies_option : std::uint8_t {
        exclude_prerelease, // Exclude prerelease identifiers in range checks
        include_prerelease  // Include prerelease identifiers in range checks
    };

    namespace detail {

        // Structure representing a range comparator
        struct range_comparator {
            enum struct op_t {
                equal,
                less,
                less_equal,
                greater,
                greater_equal
            };
            op_t op;
            version ver;

            bool satisfies(
                const version& v, comparators_option opt) const noexcept
            {
                switch (op) {
                case op_t::equal:
                    return equal_to(v, ver, opt);
                case op_t::less:
                    return less(v, ver, opt);
                case op_t::less_equal:
                    return less_equal(v, ver, opt);
                case op_t::greater:
                    return greater(v, ver, opt);
                case op_t::greater_equal:
                    return greater_equal(v, ver, opt);
                default:
                    SEMVER_THROW("Unexpected range operator");
                }
            }
        };

        // Structure representing a token in range parsing
        struct range_token {
            enum struct type_t {
                none,
                number,
                operator_t,
                dot,
                hyphen,
                logical_or,
                tilde,
                caret,
                prerelease,
                end_of_line
            };
            type_t type = type_t::none;
            std::uint64_t number = 0;
            range_comparator::op_t op = range_comparator::op_t::equal;
            std::string identifier;
        };

        // Lexer for parsing range expressions
        class range_lexer {
            std::string_view text;
            std::size_t pos;

        public:
            explicit range_lexer(std::string_view t) noexcept
                : text { t }
                , pos { 0 }
            {
            }

            range_token get_next_token(parse_error& err) noexcept
            {
                while (!end_of_line()) {
                    if (::semver::detail::is_space(text[pos])) {
                        advance(1);
                        continue;
                    }
                    if (pos + 1 < text.length()
                        && ::semver::detail::is_logical_or(text[pos])
                        && text[pos + 1] == '|') {
                        advance(2);
                        return { range_token::type_t::logical_or };
                    }
                    if (text[pos] == '~') {
                        advance(1);
                        return { range_token::type_t::tilde };
                    }
                    if (text[pos] == '^') {
                        advance(1);
                        return { range_token::type_t::caret };
                    }
                    if (::semver::detail::is_operator(text[pos])) {
                        auto op = get_operator();
                        return { range_token::type_t::operator_t, 0, op };
                    }
                    if (::semver::detail::is_digit(text[pos])) {
                        auto number = get_number(err);
                        if (err != parse_error::none)
                            return { range_token::type_t::end_of_line };
                        return { range_token::type_t::number, number };
                    }
                    if (::semver::detail::is_dot(text[pos])) {
                        advance(1);
                        return { range_token::type_t::dot };
                    }
                    if (::semver::detail::is_hyphen(text[pos])) {
                        advance(1);
                        return { range_token::type_t::hyphen };
                    }
                    if (::semver::detail::is_alphanumeric(text[pos])) {
                        std::string id;
                        if (auto next = ::semver::detail::from_chars_identifier(
                                text.data() + pos, text.data() + text.length(),
                                id, err);
                            next) {
                            advance(next - (text.data() + pos));
                            return { range_token::type_t::prerelease, 0,
                                range_comparator::op_t::equal, id };
                        }
                        return { range_token::type_t::end_of_line };
                    }
                    err = parse_error::invalid_character;
                    return { range_token::type_t::end_of_line };
                }
                return { range_token::type_t::end_of_line };
            }

            bool end_of_line() const noexcept
            {
                return pos >= text.length();
            }

            void advance(std::size_t i) noexcept
            {
                pos += i;
            }

        private:
            range_comparator::op_t get_operator() noexcept
            {
                if (text[pos] == '<') {
                    advance(1);
                    if (pos < text.length() && text[pos] == '=') {
                        advance(1);
                        return range_comparator::op_t::less_equal;
                    }
                    return range_comparator::op_t::less;
                } else if (text[pos] == '>') {
                    advance(1);
                    if (pos < text.length() && text[pos] == '=') {
                        advance(1);
                        return range_comparator::op_t::greater_equal;
                    }
                    return range_comparator::op_t::greater;
                } else if (text[pos] == '=') {
                    advance(1);
                    return range_comparator::op_t::equal;
                }
                return range_comparator::op_t::equal;
            }

            std::uint64_t get_number(parse_error& err) noexcept
            {
                const auto first = text.data() + pos;
                const auto last = text.data() + text.length();
                std::uint64_t n = 0;
                if (auto next
                    = ::semver::detail::from_chars(first, last, n, err);
                    next) {
                    advance(next - first);
                    return n;
                }
                return 0;
            }
        };

        // Parser for range expressions
        class range_parser {
            range_lexer lexer;
            range_token current_token;
            parse_error error = parse_error::none;

        public:
            explicit range_parser(std::string_view str)
                : lexer { str }
                , current_token { range_token::type_t::none }
            {
                advance_token(range_token::type_t::none);
            }

            std::vector<std::vector<range_comparator>> parse()
            {
                std::vector<std::vector<range_comparator>> ranges;
                std::vector<range_comparator> current_group;

                while (current_token.type != range_token::type_t::end_of_line) {
                    if (current_token.type == range_token::type_t::logical_or) {
                        if (!current_group.empty()) {
                            ranges.push_back(std::move(current_group));
                            current_group.clear();
                        }
                        advance_token(range_token::type_t::logical_or);
                        continue;
                    }

                    auto comps = parse_comparator();
                    if (error != parse_error::none) {
                        SEMVER_THROW(
                            std::format("Range parsing failed: error code {}",
                                static_cast<int>(error)));
                    }
                    current_group.insert(
                        current_group.end(), comps.begin(), comps.end());
                }

                if (!current_group.empty()) {
                    ranges.push_back(std::move(current_group));
                }

                if (ranges.empty()) {
                    SEMVER_THROW("Empty range specification");
                }

                return ranges;
            }

        private:
            void advance_token(range_token::type_t expected)
            {
                if (current_token.type != expected
                    && expected != range_token::type_t::none) {
                    error = parse_error::invalid_format;
                    return;
                }
                current_token = lexer.get_next_token(error);
            }

            std::vector<range_comparator> parse_comparator()
            {
                std::vector<range_comparator> comps;
                range_comparator::op_t op = range_comparator::op_t::equal;
                bool is_tilde = false;
                bool is_caret = false;

                if (current_token.type == range_token::type_t::tilde) {
                    is_tilde = true;
                    advance_token(range_token::type_t::tilde);
                } else if (current_token.type == range_token::type_t::caret) {
                    is_caret = true;
                    advance_token(range_token::type_t::caret);
                } else if (current_token.type
                    == range_token::type_t::operator_t) {
                    op = current_token.op;
                    advance_token(range_token::type_t::operator_t);
                }

                auto ver = parse_version();
                if (error != parse_error::none) {
                    return comps;
                }

                if (is_tilde) {
                    // ~1.2.3 = >=1.2.3 <1.3.0
                    comps.push_back(
                        { range_comparator::op_t::greater_equal, ver });
                    version upper { ver.major, ver.minor + 1, 0 };
                    comps.push_back({ range_comparator::op_t::less, upper });
                } else if (is_caret) {
                    // ^1.2.3 = >=1.2.3 <2.0.0
                    comps.push_back(
                        { range_comparator::op_t::greater_equal, ver });
                    version upper { ver.major + 1, 0, 0 };
                    comps.push_back({ range_comparator::op_t::less, upper });
                } else if (current_token.type == range_token::type_t::hyphen) {
                    advance_token(range_token::type_t::hyphen);
                    auto end_ver = parse_version();
                    if (error != parse_error::none) {
                        return comps;
                    }
                    comps.push_back(
                        { range_comparator::op_t::greater_equal, ver });
                    comps.push_back(
                        { range_comparator::op_t::less_equal, end_ver });
                } else {
                    comps.push_back({ op, ver });
                }

                return comps;
            }

            version parse_version()
            {
                std::uint64_t major = parse_number();
                if (error != parse_error::none)
                    return {};

                advance_token(range_token::type_t::dot);
                if (error != parse_error::none)
                    return {};

                std::uint64_t minor = parse_number();
                if (error != parse_error::none)
                    return {};

                advance_token(range_token::type_t::dot);
                if (error != parse_error::none)
                    return {};

                std::uint64_t patch = parse_number();
                if (error != parse_error::none)
                    return {};

                std::vector<std::string> prerelease;
                if (current_token.type == range_token::type_t::hyphen) {
                    advance_token(range_token::type_t::hyphen);
                    while (
                        current_token.type == range_token::type_t::prerelease) {
                        prerelease.push_back(current_token.identifier);
                        advance_token(range_token::type_t::prerelease);
                        if (current_token.type == range_token::type_t::dot) {
                            advance_token(range_token::type_t::dot);
                        } else {
                            break;
                        }
                    }
                }

                return { major, minor, patch, prerelease, {} };
            }

            std::uint64_t parse_number()
            {
                if (current_token.type != range_token::type_t::number) {
                    error = parse_error::invalid_format;
                    return 0;
                }
                auto number = current_token.number;
                advance_token(range_token::type_t::number);
                return number;
            }
        };

        // Range class for version range checks
        class range {
            std::vector<std::vector<range_comparator>> ranges;

        public:
            explicit range(std::string_view str)
                : ranges { range_parser { str }.parse() }
            {
            }

            bool satisfies(
                const version& ver, satisfies_option option) const noexcept
            {
                comparators_option comp_opt
                    = option == satisfies_option::exclude_prerelease
                    ? comparators_option::exclude_prerelease
                    : comparators_option::include_prerelease;

                for (const auto& group : ranges) {
                    bool group_satisfies = true;
                    for (const auto& comp : group) {
                        if (!comp.satisfies(ver, comp_opt)) {
                            group_satisfies = false;
                            break;
                        }
                    }
                    if (group_satisfies)
                        return true;
                }
                return false;
            }
        };

    } // namespace semver::range::detail

    // Check if a version satisfies a range expression
    inline bool satisfies(const version& ver, std::string_view str,
        satisfies_option option = satisfies_option::exclude_prerelease)
    {
        try {
            return detail::range { str }.satisfies(ver, option);
        } catch (...) {
            return false;
        }
    }

} // namespace semver::range

// Library version constant
inline const auto semver_version = version { SEMVER_VERSION_MAJOR,
    SEMVER_VERSION_MINOR, SEMVER_VERSION_PATCH };

} // namespace semver

#endif // SEMVER_HPP
