#ifndef RPC_CLIENT_H
#define RPC_CLIENT_H

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class RpcClient {
public:
    static json sendRpcRequest(const std::string& method, const json& params);
    static json callContract(
        const std::string& contractAddress, const std::string& data);
};

#endif // RPC_CLIENT_H
