#include "config.h"

const std::string Config::RPC_HOST = "arb1.arbitrum.io";
const std::string Config::RPC_PATH = "/rpc";
const std::string Config::UNISWAP_V3_FACTORY
    = "0x1F98431c8aD98523631AE4a59f267346ea31F984";
const std::vector<uint32_t> Config::STANDARD_FEES = { 100, 500, 3000, 10000 };
const uint64_t Config::MIN_LIQUIDITY = 1000000;
const long double Config::MIN_ARBITRAGE_PROFIT = 0.001L;

const std::string Config::GET_POOL_FUNC = "0x1698ee82";
const std::string Config::LIQUIDITY_FUNC = "0x1a686502";
const std::string Config::SLOT0_FUNC = "0x3850c7bd";
const std::string Config::TOKEN0_FUNC = "0x0dfe1681";
const std::string Config::TOKEN1_FUNC = "0xd21220a7";
const std::string Config::FEE_FUNC = "0xddca3f43";
