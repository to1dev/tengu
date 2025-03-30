#include "BlockchainMonitor.h"

namespace Daitengu::Wallets {

BlockchainMonitor::BlockchainMonitor(
    ChainType chainType, ProviderType initialProvider, QObject* parent)
{
}

BlockchainMonitor::~BlockchainMonitor()
{
}

QCoro::Task<void> BlockchainMonitor::setAddress(const QString& address)
{
    return QCoro::Task<void>();
}

QCoro::Task<bool> BlockchainMonitor::connect()
{
    return QCoro::Task<bool>();
}

QCoro::Task<void> BlockchainMonitor::disconnect()
{
    return QCoro::Task<void>();
}

QCoro::Task<void> BlockchainMonitor::refreshBalance()
{
    return QCoro::Task<void>();
}

QCoro::Task<void> BlockchainMonitor::refreshTokens()
{
    return QCoro::Task<void>();
}

bool BlockchainMonitor::isConnected() const
{
    return false;
}

bool BlockchainMonitor::isConnecting() const
{
    return false;
}

QCoro::Task<bool> BlockchainMonitor::isValidAddress(const QString& address)
{
    return QCoro::Task<bool>();
}

QCoro::Task<bool> BlockchainMonitor::switchProvider(ProviderType newProvider)
{
    return QCoro::Task<bool>();
}

QList<ProviderType> BlockchainMonitor::availableProviders() const
{
    return QList<ProviderType>();
}

QCoro::Task<void> BlockchainMonitor::setAutoRefreshInterval(int milliseconds)
{
    return QCoro::Task<void>();
}

void BlockchainMonitor::onProviderBalanceChanged(
    const QString& address, const BalanceResult& balance)
{
}

void BlockchainMonitor::onProviderTokensChanged(
    const QString& address, const TokenList& tokens)
{
}

void BlockchainMonitor::onProviderConnectionChanged(bool connected)
{
}

void BlockchainMonitor::onProviderError(const QString& message)
{
}

void BlockchainMonitor::onAutoRefreshTimer()
{
}

QCoro::Task<void> BlockchainMonitor::setupProviderConnections()
{
    return QCoro::Task<void>();
}

QString BlockchainMonitor::balanceResultToString(
    const BalanceResult& balance) const
{
    return QString();
}
}
