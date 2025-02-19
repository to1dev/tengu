#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <string>
#include <vector>

namespace Daitengu::Wallets {

enum class ChainType {
    BITCOIN,
    ETHEREUM,
    SOLANA,
    BNBCHAIN,
};

enum class NetworkType {
    MAINNET,
    TESTNET,
    DEVNET,
};

struct ChainConfig {
    std::uint32_t coinType;
    std::string curveName;
    bool useHardenedChange;
    std::vector<std::uint32_t> basePath;
};

struct ChainNetwork {
    NetworkType type;
    std::uint8_t addressPrefix;
    std::string networkName;
    std::string bech32Prefix;
};

}
#endif // TYPES_H
