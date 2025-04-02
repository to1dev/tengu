#include "ProviderFactory.h"

#include "Bitcoin/MempoolSpaceProvider.h"

namespace Daitengu::Wallets {

BlockchainProvider* ProviderFactory::createProvider(
    ChainType chainType, ProviderType providerType, QObject* parent)
{
    if (providerType == ProviderType::NONE) {
        providerType = getDefaultProvider(chainType);
    }

    if (chainType == ChainType::BITCOIN) {
        switch (providerType) {
        case ProviderType::MEMPOOL_SPACE:
            return new MempoolSpaceProvider(parent);
        default:
            spdlog::warn("Unknown Bitcoin provider type: {}",
                static_cast<int>(providerType));
            return nullptr;
        }
    }

    spdlog::warn("Unknown chain type: {}", static_cast<int>(chainType));

    return nullptr;
}

QList<ProviderType> ProviderFactory::getAvailableProviders(ChainType chainType)
{
    QList<ProviderType> providers;

    switch (chainType) {
    case ChainType::BITCOIN:
        providers << ProviderType::MEMPOOL_SPACE;
        break;
    case ChainType::ETHEREUM:
        providers << ProviderType::ETHERSCAN;
        break;
    default:
        spdlog::warn("Unknown chain type for available providers: {}",
            static_cast<int>(chainType));
        break;
    }

    return providers;
}

QString ProviderFactory::getProviderName(ProviderType type)
{
    switch (type) {
    case ProviderType::NONE:
        return "None";
    case ProviderType::MEMPOOL_SPACE:
        return "mempool.space";
    case ProviderType::ETHERSCAN:
        return "Etherscan";
    default:
        return "Unknown Provider";
    }
}

QString ProviderFactory::getProviderDescription(ProviderType type)
{
    switch (type) {
    case ProviderType::NONE:
        return "No provider selected";
    case ProviderType::MEMPOOL_SPACE:
        return "Open-source Bitcoin explorer";
    case ProviderType::ETHERSCAN:
        return "Ethereum blockchain explorer";
    default:
        return "Unknown provider";
    }
}

ProviderType ProviderFactory::getDefaultProvider(ChainType chainType)
{
    switch (chainType) {
    case ChainType::BITCOIN:
        return ProviderType::MEMPOOL_SPACE;
    case ChainType::ETHEREUM:
        return ProviderType::ETHERSCAN;
    case ChainType::SOLANA:
        return ProviderType::SOLSCAN;
    default:
        spdlog::warn("Unknown chain type for default provider: {}",
            static_cast<int>(chainType));
        return ProviderType::MEMPOOL_SPACE;
    }
}
}
