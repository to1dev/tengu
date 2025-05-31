#include <iostream>
#include <string>

#include <httplib.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Arbitrum官方主网RPC
const std::string RPC_URL = "https://arb1.arbitrum.io/rpc";

// 池子合约地址（Arbitrum主网，需替换为实际地址）
const std::string UNISWAP_V3_ETH_USDC
    = "0xC31E54c7a869B9FcBEcc14363CF510d1c41fa443"; // 示例地址
const std::string UNISWAP_V3_USDC_DAI
    = "0xd37Af656Abf91c7f548FfFC0133175b5e4d3d5e6"; // 示例地址
const std::string SUSHISWAP_DAI_ETH
    = "0xC3D03e4F041Fd4cD388c549Ee2A29a9E5075882f"; // 示例地址

int main()
{
    std::cout << "fuck off" << std::endl;
}
