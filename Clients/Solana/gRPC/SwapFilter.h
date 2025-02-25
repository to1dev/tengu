#ifndef SWAPFILTER_H
#define SWAPFILTER_H

#include "TransactionFilter.h"

namespace Daitengu::Clients::Solana {

class SwapFilter : public TransactionFilter {
public:
    explicit SwapFilter(const std::unordered_set<std::string>& smartWallets);

    void processTransaction(
        const geyser::SubscribeUpdateTransaction& tx) override;

private:
    struct TokenBalance {
        std::string owner;
        std::string mint;
        double amount;
    };

    std::vector<TokenBalance> getPreBalances(
        const geyser::SubscribeUpdateTransaction& tx);
    std::vector<TokenBalance> getPostBalances(
        const geyser::SubscribeUpdateTransaction& tx);

    bool isSmartWallet(const std::string& address) const;

    void reportSwap(const std::string& wallet, const std::string& tokenMint,
        double tokenChange,
        const std::unordered_map<std::string, double>& wsolChanges);

private:
    const std::string WSOL_MINT = "So11111111111111111111111111111111111111112";
    std::unordered_set<std::string> smartWallets_;
};

}
#endif // SWAPFILTER_H
