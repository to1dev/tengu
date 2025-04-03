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

#pragma once

#include <spdlog/spdlog.h>

#include "sol/sol.hpp"

#include "Wallets/Monitor/MonitorManager.h"

#include "ScriptEngine.h"

using namespace Daitengu::Wallets;

namespace Daitengu::Scripting {

class LuaScriptEngine : public ScriptEngine {
public:
    LuaScriptEngine();
    ~LuaScriptEngine() override = default;

    bool initialize() override;
    bool loadScript(const std::string& filePath) override;
    bool execute(const std::string& script) override;
    std::any callFunction(const std::string& funcName,
        const std::vector<std::any>& args) override;
    void registerFunction(const std::string& name,
        std::function<std::any(const std::vector<std::any>&)> func) override;
    void setCallback(const std::string& name,
        std::function<void(const std::vector<std::any>&)> callback) override;

protected:
    void registerObjectImpl(
        const std::string& name, void* obj, std::type_index type) override;

private:
    sol::state lua_;
    std::unordered_map<std::string, sol::protected_function> luaCallbacks_;
};

}
