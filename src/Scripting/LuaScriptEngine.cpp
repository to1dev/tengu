// SPDX-License-Identifier: AGPL-3.0-or-later
/*
 * Copyright (C) 2025 to1dev <https://arc20.me/to1dev>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "LuaScriptEngine.h"

namespace Daitengu::Scripting {

LuaScriptEngine::LuaScriptEngine()
{
    initialize();
}

LuaScriptEngine::~LuaScriptEngine()
{
    stopScript();
}

bool LuaScriptEngine::initialize()
{
    std::lock_guard<std::mutex> lock { luaMutex_ };
    try {
        lua_.open_libraries(sol::lib::base, sol::lib::package);
        scriptLua_.open_libraries(sol::lib::base, sol::lib::package);
        spdlog::info("LuaJIT script engine initialized");
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to initialize LuaJIT: {}", e.what());
        return false;
    }
}

bool LuaScriptEngine::loadScript(std::string_view filePath)
{
    std::lock_guard<std::mutex> lock { luaMutex_ };
    try {
        auto result = lua_.script_file(filePath.data());
        if (!result.valid()) {
            sol::error err = result;
            spdlog::error(
                "Failed to load Lua script {}: {}", filePath, err.what());
            return false;
        }
        spdlog::info("Loaded Lua script: {}", filePath);
        return true;
    } catch (const std::exception& e) {
        spdlog::error(
            "Exception while loading Lua script {}: {}", filePath, e.what());
        return false;
    }
}

bool LuaScriptEngine::execute(std::string_view script)
{
    std::lock_guard<std::mutex> lock { luaMutex_ };
    try {
        auto result = lua_.script(script);
        if (!result.valid()) {
            sol::error err = result;
            spdlog::error("Failed to execute Lua script: {}", err.what());
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Exception while executing Lua script: {}", e.what());
        return false;
    }
}

std::any LuaScriptEngine::callFunction(
    std::string_view funcName, const std::vector<std::any>& args)
{
    std::lock_guard<std::mutex> lock { luaMutex_ };
    sol::protected_function func = lua_[funcName];
    if (!func.valid()) {
        spdlog::warn("Lua function {} not found", funcName);
        return std::any {};
    }

    std::vector<sol::object> luaArgs;
    for (const auto& arg : args) {
        if (arg.type() == typeid(int)) {
            luaArgs.push_back(sol::make_object(lua_, std::any_cast<int>(arg)));
        } else if (arg.type() == typeid(std::string)) {
            luaArgs.push_back(
                sol::make_object(lua_, std::any_cast<std::string>(arg)));
        } else {
            spdlog::warn(
                "Unsupported argument type for Lua function {}", funcName);
        }
    }

    auto result = func.call(sol::as_args(luaArgs));
    if (!result.valid()) {
        sol::error err = result;
        spdlog::error(
            "Failed to call Lua function {}: {}", funcName, err.what());
        return std::any {};
    }

    if (result.get_type() == sol::type::number) {
        return std::any(result.get<double>());
    } else if (result.get_type() == sol::type::string) {
        return std::any(result.get<std::string>());
    }

    return std::any {};
}

void LuaScriptEngine::registerFunction(std::string_view name,
    std::function<std::any(const std::vector<std::any>&)> func)
{
    std::lock_guard<std::mutex> lock { luaMutex_ };
    lua_.set_function(name, [func](sol::variadic_args va) -> sol::object {
        std::vector<std::any> args;
        for (auto v : va) {
            if (v.get_type() == sol::type::number) {
                args.emplace_back(v.as<double>());
            } else if (v.get_type() == sol::type::string) {
                args.emplace_back(v.as<std::string>());
            }
        }
        return sol::make_object(va.lua_state(), func(args));
    });

    scriptLua_.set_function(name, [func](sol::variadic_args va) -> sol::object {
        std::vector<std::any> args;
        for (auto v : va) {
            if (v.get_type() == sol::type::number) {
                args.emplace_back(v.as<double>());
            } else if (v.get_type() == sol::type::string) {
                args.emplace_back(v.as<std::string>());
            }
        }
        return sol::make_object(va.lua_state(), func(args));
    });

    spdlog::info("Registered C++ function to Lua: {}", name);
}

void LuaScriptEngine::setCallback(std::string_view name,
    std::function<void(const std::vector<std::any>&)> callback)
{
    std::lock_guard<std::mutex> lock { luaMutex_ };
    callbacks_[name] = callback;
    lua_.set_function(name, [this, name](sol::variadic_args va) {
        std::vector<std::any> args;
        for (auto v : va) {
            if (v.get_type() == sol::type::number) {
                args.emplace_back(v.as<double>());
            } else if (v.get_type() == sol::type::string) {
                args.emplace_back(v.as<std::string>());
            }
        }
        if (callbacks_.count(name)) {
            callbacks_[name](args);
        }
    });

    scriptLua_.set_function(name, [this, name](sol::variadic_args va) {
        std::vector<std::any> args;
        for (auto v : va) {
            if (v.get_type() == sol::type::number) {
                args.emplace_back(v.as<double>());
            } else if (v.get_type() == sol::type::string) {
                args.emplace_back(v.as<std::string>());
            }
        }
        if (callbacks_.count(name)) {
            callbacks_[name](args);
        }
    });
}

void LuaScriptEngine::startScript(std::string_view scriptOrFile)
{
    std::lock_guard<std::mutex> lock { luaMutex_ };

    stopScript();

    running_ = true;
    scriptContent_ = std::string { scriptOrFile };
    scriptThread_ = std::jthread { &LuaScriptEngine::runScript, this,
        scriptContent_.value() };
    spdlog::info(
        "Script thread started with content: {}", scriptOrFile.substr(0, 50));
}

void LuaScriptEngine::stopScript()
{
    std::lock_guard<std::mutex> lock { luaMutex_ };

    if (scriptThread_.joinable()) {
        running_ = false;
        scriptThread_.request_stop();
        scriptThread_.join();
        spdlog::info("Script thread stopped");
    }
    scriptContent_.reset();
}

bool LuaScriptEngine::isScriptRunning() const noexcept
{
    return running_ && scriptThread_.joinable();
}

void LuaScriptEngine::registerObjectImpl(
    std::string_view name, void* obj, std::type_index type)
{
    std::lock_guard<std::mutex> lock { luaMutex_ };
    if (type == std::type_index(typeid(Monitor))) {
        auto* manager = static_cast<Monitor*>(obj);
        lua_.new_usertype<Monitor>(
            name, "setAddress",
            [manager](const std::string& address) {
                manager->setAddress(
                    ChainType::UNKNOWN, QString::fromStdString(address));
            },
            "refreshBalance", [manager]() { manager->refresh(); },
            "setAutoRefreshInterval", &Monitor::setRefreshInterval);

        auto scriptUsertype = scriptLua_.new_usertype<Monitor>(name);
        scriptUsertype["setAddress"] = [manager](const std::string& address) {
            manager->setAddress(
                ChainType::UNKNOWN, QString::fromStdString(address));
        };
        scriptUsertype["refreshBalance"] = [manager]() { manager->refresh(); };
        scriptUsertype["setAutoRefreshInterval"] = &Monitor::setRefreshInterval;

        spdlog::info("Registered MonitorManager to Lua as {}", name);
    }
}

void LuaScriptEngine::runScript(
    std::stop_token stopToken, std::string scriptOrFile)
{
    // Need more criteria
    bool isFile = scriptOrFile.ends_with(".lua");

    while (!stopToken.stop_requested() && running_) {
        try {
            sol::protected_function_result result;
            if (isFile) {
                result = scriptLua_.script_file(scriptOrFile);
            } else {
                result = scriptLua_.script(scriptOrFile);
            }

            if (!result.valid()) {
                sol::error err = result;
                spdlog::error("Script execution failed: {}", err.what());
                running_ = false;
                break;
            }

            sol::protected_function loop = scriptLua_["loop"];
            if (!loop.valid() || !loop.call<bool>()) {
                running_ = false;
                break;
            }
        } catch (const std::exception& e) {
            spdlog::error("Script thread exception: {}", e.what());
            running_ = false;
            break;
        }
    }
    spdlog::info("Script thread completed or stopped");
}
}

/*
auto engine = std::make_unique<LuaScriptEngine>();

engine->registerFunction(
    "logMessage", [](const std::vector<std::any>& args) -> std::any {
        if (!args.empty() && args[0].type() == typeid(std::string)) {
            spdlog::info("Lua called logMessage: {}",
                std::any_cast<std::string>(args[0]));
        }
        return std::any {};
    });

MonitorManager manager;
engine->registerObject("walletMonitor", &manager);

engine->setCallback("onBalanceUpdate", [](const std::vector<std::any>& args) {
    if (args.size() >= 3) {
        spdlog::info("Balance updated: address={}, balance={}, chain={}",
            std::any_cast<std::string>(args[0]),
            std::any_cast<std::string>(args[1]), std::any_cast<int>(args[2]));
    }
});

engine->execute(R"(
        walletMonitor:setAddress("1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa")
        walletMonitor:refreshBalance()
        logMessage("Hello from Lua!")
        onBalanceUpdate("1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa", "50.0", 0)
    )");

engine->registerFunction("getSystemTime", []() -> std::any {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch())
                  .count();
    return std::any(static_cast<double>(ms));
});

*/
