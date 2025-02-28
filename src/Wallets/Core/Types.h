#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace Daitengu::Wallets {

enum class ChainType {
    BITCOIN,
    ETHEREUM,
    SOLANA,
    TRON,
    BNBCHAIN,
};

struct Network {
    enum class Type {
        MAINNET,
        TESTNET,
        DEVNET,
    };
};

enum class AddressEncoding {
    BASE58CHECK,
    BECH32,
    BECH32M,
    HEX,
    BASE58,
    SS58,
    CUSTOM,
};

struct ChainConfig {
    std::uint32_t coinType;
    std::string curveName;
    bool useHardenedChange;
    std::vector<std::uint32_t> basePath;
};

struct ChainNetwork {
    Network::Type type;
    std::string networkName;
    std::uint64_t chainId;
    AddressEncoding addressEncoding;
    std::string addressPrefix;
    std::vector<std::uint8_t> prefixBytes;
    std::string hrp;
    std::uint8_t versionByte;
    bool supportsEip55;
    std::function<std::string(const std::vector<std::uint8_t>&)> customEncoder;
    std::function<bool(const std::string&)> customValidator;
};

}
#endif // TYPES_H
