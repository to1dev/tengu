#ifndef TRANSACTIONFILTER_H
#define TRANSACTIONFILTER_H

#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "geyser/geyser.grpc.pb.h"

namespace Daitengu::Clients::Solana {

class TransactionFilter {
public:
    virtual ~TransactionFilter() = default;

    virtual void processTransaction(
        const geyser::SubscribeUpdateTransaction& tx)
        = 0;
};

}
#endif // TRANSACTIONFILTER_H
