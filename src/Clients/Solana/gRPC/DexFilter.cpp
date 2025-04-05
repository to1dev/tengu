#include "DexFilter.h"

namespace Daitengu::Clients::Solana::gRPC {

DexFilter::DexFilter(std::unordered_set<std::string> dexPrograms)
    : dexPrograms_(std::move(dexPrograms))
{
}

void DexFilter::processTransaction(const geyser::SubscribeUpdateTransaction& tx)
{
    const auto& transaction = tx.transaction();
    const auto& accountKeys
        = transaction.transaction().message().account_keys();
    for (const auto& instr :
        transaction.transaction().message().instructions()) {
        uint32_t programIdIndex = instr.program_id_index();
        if (programIdIndex < static_cast<uint32_t>(accountKeys.size())) {
            const std::string& programId = accountKeys[programIdIndex];
            if (dexPrograms_.contains(programId)) {
                spdlog::info("[DexFilter] DEX transaction detected: {}",
                    transaction.signature());
                break;
            }
        } else {
            spdlog::warn("Invalid program_id_index {} for transaction {}",
                programIdIndex, transaction.signature());
        }
    }
}

void DexFilter::updateConfig(const std::string& config)
{
    try {
        auto json = nlohmann::json::parse(config);
        dexPrograms_.clear();
        for (const auto& program : json["dex_programs"]) {
            dexPrograms_.insert(program.get<std::string>());
        }
        spdlog::info(
            "[DexFilter] Updated DEX programs: {}", dexPrograms_.size());
    } catch (const std::exception& e) {
        spdlog::error("[DexFilter] Failed to update config: {}", e.what());
    }
}
}
