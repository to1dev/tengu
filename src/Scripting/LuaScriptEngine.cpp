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

bool LuaScriptEngine::initialize()
{
    try {
        lua_.open_libraries(sol::lib::base, sol::lib::package);
        spdlog::info("LuaJIT script engine initialized");
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to initialize LuaJIT: {}", e.what());
        return false;
    }
}

bool LuaScriptEngine::loadScript(const std::string& filePath)
{
    try {
        auto result = lua_.script_file(filePath);
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

bool LuaScriptEngine::execute(const std::string& script)
{
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
    const std::string& funcName, const std::vector<std::any>& args)
{
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

void LuaScriptEngine::registerFunction(const std::string& name,
    std::function<std::any(const std::vector<std::any>&)> func)
{
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

    spdlog::info("Registered C++ function to Lua: {}", name);
}

void LuaScriptEngine::setCallback(const std::string& name,
    std::function<void(const std::vector<std::any>&)> callback)
{
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
}

void LuaScriptEngine::registerObjectImpl(
    const std::string& name, void* obj, std::type_index type)
{
    if (type == std::type_index(typeid(MonitorManager))) {
        auto* manager = static_cast<MonitorManager*>(obj);
        lua_.new_usertype<MonitorManager>(
            name, "setAddress",
            [manager](const std::string& address) {
                manager->setAddress(QString::fromStdString(address));
            },
            "refreshBalance", [manager]() { manager->refreshBalance(); },
            "setAutoRefreshInterval", &MonitorManager::setAutoRefreshInterval);
        spdlog::info("Registered MonitorManager to Lua as {}", name);
    }
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
*/
