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
    BNBCHAIN,
};

enum class NetworkType {
    MAINNET,
    TESTNET,
    DEVNET,
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

struct WalletRecord {
    int id = 0;
    ChainType chainType;
    NetworkType networkType;
    std::string name;
    std::string mnemonic;
    std::string passphrase;
    std::string masterPrivateKey;
    std::string extendedPublicKey;
};

struct AddressRecord {
    int id = 0;
    int walletId = 0;
    std::string address;
    std::string derivationPath;
    std::string privateKey;
    std::string publicKey;
};

struct ChainConfig {
    std::uint32_t coinType;
    std::string curveName;
    bool useHardenedChange;
    std::vector<std::uint32_t> basePath;
};

struct ChainNetwork {
    NetworkType type;
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
