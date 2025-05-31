#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "rpc_client.h"
#include "config.h"

#include <stdexcept>

#include <httplib.h>

json RpcClient::sendRpcRequest(const std::string& method, const json& params)
{
    httplib::SSLClient cli(Config::RPC_HOST);
    cli.enable_server_certificate_verification(false);
    cli.set_connection_timeout(30, 0);

    json request = { { "jsonrpc", "2.0" }, { "method", method },
        { "params", params }, { "id", 1 } };

    auto res = cli.Post(Config::RPC_PATH, request.dump(), "application/json");

    if (!res || res->status != 200) {
        throw std::runtime_error("RPC request failed");
    }

    auto response = json::parse(res->body);
    if (response.contains("error")) {
        throw std::runtime_error("RPC error: " + response["error"].dump());
    }

    return response["result"];
}

json RpcClient::callContract(
    const std::string& contractAddress, const std::string& data)
{
    json params = json::array();
    params.push_back({ { "to", contractAddress }, { "data", data } });
    params.push_back("latest");

    return sendRpcRequest("eth_call", params);
}
