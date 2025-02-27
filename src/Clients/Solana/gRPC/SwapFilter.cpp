#include "SwapFilter.h"

namespace Daitengu::Clients::Solana {

SwapFilter::SwapFilter(const std::unordered_set<std::string>& smartWallets)
    : smartWallets_(smartWallets)
{
}

void SwapFilter::processTransaction(
    const geyser::SubscribeUpdateTransaction& tx)
{
    auto preBalances = getPreBalances(tx);
    auto postBalances = getPostBalances(tx);

    std::unordered_map<std::string, double> wsolChanges;
    for (const auto& pre : preBalances) {
        if (pre.mint == WSOL_MINT) {
            for (const auto& post : postBalances) {
                if (post.owner == pre.owner && post.mint == WSOL_MINT) {
                    double change = post.amount - pre.amount;
                    if (std::fabs(change) > 1e-6) {
                        wsolChanges[pre.owner] = change;
                    }
                }
            }
        }
    }

    for (const auto& pre : preBalances) {
        if (isSmartWallet(pre.owner) && pre.mint != WSOL_MINT) {
            for (const auto& post : postBalances) {
                if (post.owner == pre.owner && post.mint == pre.mint) {
                    double tokenChange = post.amount - pre.amount;
                    if (std::fabs(tokenChange) > 1e-6) {
                        reportSwap(
                            pre.owner, pre.mint, tokenChange, wsolChanges);
                    }
                }
            }
        }
    }
}

bool SwapFilter::isSmartWallet(const std::string& address) const
{
    return (smartWallets_.find(address) != smartWallets_.end());
}

void SwapFilter::reportSwap(const std::string& wallet,
    const std::string& tokenMint, double tokenChange,
    const std::unordered_map<std::string, double>& wsolChanges)
{
    double wsolChange = 0.0;
    for (const auto& [owner, change] : wsolChanges) {
        if (((tokenChange < 0) && (change > 0))
            || ((tokenChange > 0) && (change < 0))) {
            wsolChange = change;
            break;
        }
    }

    if (tokenChange < 0) {
        std::cout << "[SwapFilter] Wallet " << wallet << " sold "
                  << std::fabs(tokenChange) << " of token " << tokenMint
                  << " for " << wsolChange << " WSOL\n";
    } else {
        std::cout << "[SwapFilter] Wallet " << wallet << " bought "
                  << tokenChange << " of token " << tokenMint << " using "
                  << std::fabs(wsolChange) << " WSOL\n";
    }
    std::cout << "-------------------" << std::endl;
}

std::vector<SwapFilter::TokenBalance> SwapFilter::getPostBalances(
    const geyser::SubscribeUpdateTransaction& tx)
{
    std::vector<TokenBalance> balances;
    const auto& meta = tx.transaction().meta();

    for (const auto& tb : meta.post_token_balances()) {
        balances.push_back(
            { tb.owner(), tb.mint(), tb.ui_token_amount().ui_amount() });
    }
    return balances;
}

std::vector<SwapFilter::TokenBalance> SwapFilter::getPreBalances(
    const geyser::SubscribeUpdateTransaction& tx)
{
    std::vector<TokenBalance> balances;
    const auto& meta = tx.transaction().meta();

    for (const auto& tb : meta.pre_token_balances()) {
        balances.push_back(
            { tb.owner(), tb.mint(), tb.ui_token_amount().ui_amount() });
    }
    return balances;
}

}
