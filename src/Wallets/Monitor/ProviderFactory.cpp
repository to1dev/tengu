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
        case ProviderType::BLOCKCHAIN_COM:
            qCWarning(bcMonitor) << "Provider not implemented: BLOCKCHAIN_COM";
            return nullptr;
        case ProviderType::BLOCKSTREAM:
            qCWarning(bcMonitor) << "Provider not implemented: BLOCKSTREAM";
            return nullptr;
        default:
            qCWarning(bcMonitor) << "Unknown Bitcoin provider type: "
                                 << static_cast<int>(providerType);
            return nullptr;
        }
    } else if (chainType == ChainType::ETHEREUM) {
        switch (providerType) {
        case ProviderType::ETHERSCAN:
            qCWarning(bcMonitor) << "Provider not implemented: ETHERSCAN";
            return nullptr;
        default:
            qCWarning(bcMonitor) << "Unknown Ethereum provider type: "
                                 << static_cast<int>(providerType);
            return nullptr;
        }
    } else if (chainType == ChainType::SOLANA) {
        switch (providerType) {
        case ProviderType::SOLSCAN:
            qCWarning(bcMonitor) << "Provider not implemented: SOLSCAN";
            return nullptr;
        default:
            qCWarning(bcMonitor) << "Unknown Solana provider type: "
                                 << static_cast<int>(providerType);
            return nullptr;
        }
    }

    qCWarning(bcMonitor) << "Unknown chain type: "
                         << static_cast<int>(chainType);
    return nullptr;
}

QList<ProviderType> ProviderFactory::getAvailableProviders(ChainType chainType)
{
    QList<ProviderType> providers;

    switch (chainType) {
    case ChainType::BITCOIN:
        providers << ProviderType::MEMPOOL_SPACE << ProviderType::BLOCKCHAIN_COM
                  << ProviderType::BLOCKSTREAM;
        break;
    case ChainType::ETHEREUM:
        providers << ProviderType::ETHERSCAN << ProviderType::INFURA
                  << ProviderType::ALCHEMY;
        break;
    case ChainType::SOLANA:
        providers << ProviderType::SOLSCAN << ProviderType::SOLANA_BEACH
                  << ProviderType::SOLANA_FM;
        break;
    default:
        qCWarning(bcMonitor) << "Unknown chain type for available providers: "
                             << static_cast<int>(chainType);
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
    case ProviderType::BLOCKCHAIN_COM:
        return "Blockchain.com";
    case ProviderType::BLOCKSTREAM:
        return "Blockstream.info";
    case ProviderType::ETHERSCAN:
        return "Etherscan";
    case ProviderType::INFURA:
        return "Infura";
    case ProviderType::ALCHEMY:
        return "Alchemy";
    case ProviderType::SOLSCAN:
        return "Solscan";
    case ProviderType::SOLANA_BEACH:
        return "Solana Beach";
    case ProviderType::SOLANA_FM:
        return "Solana FM";
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
        return "Open-source explorer with mempool visualization";
    case ProviderType::BLOCKCHAIN_COM:
        return "Popular blockchain explorer and wallet service";
    case ProviderType::BLOCKSTREAM:
        return "Bitcoin explorer by Blockstream";
    case ProviderType::ETHERSCAN:
        return "The Ethereum blockchain explorer";
    case ProviderType::INFURA:
        return "Ethereum API and node infrastructure";
    case ProviderType::ALCHEMY:
        return "Blockchain development platform";
    case ProviderType::SOLSCAN:
        return "Solana explorer with rich analytics";
    case ProviderType::SOLANA_BEACH:
        return "Solana network explorer";
    case ProviderType::SOLANA_FM:
        return "Feature-rich Solana block explorer";
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
        qCWarning(bcMonitor) << "Unknown chain type for default provider: "
                             << static_cast<int>(chainType);
        return ProviderType::MEMPOOL_SPACE;
    }
}
}
