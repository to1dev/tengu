#ifndef BASE58_H
#define BASE58_H

#include <cassert>
#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <vector>

namespace Daitengu::Utils {

/**
 * Encode a byte span as a base58-encoded string
 */
std::string EncodeBase58(std::span<const unsigned char> input);

/**
 * Decode a base58-encoded string (str) into a byte vector (vchRet).
 * return true if decoding is successful.
 */
[[nodiscard]] bool DecodeBase58(const std::string& str,
    std::vector<unsigned char>& vchRet, int max_ret_len);

}

#endif // BASE58_H
