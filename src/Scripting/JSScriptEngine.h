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

#include <fstream>
#include <memory>

#include <spdlog/spdlog.h>

#include "quickjs.h"

#include "ScriptEngine.h"

namespace Daitengu::Scripting {

class JSScriptEngine : public ScriptEngine {
public:
    JSScriptEngine();
    ~JSScriptEngine() override;

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
    static JSValue jsCallbackWrapper(JSContext* ctx, JSValueConst this_val,
        int argc, JSValueConst* argv, int magic, JSValue* func_data);
    JSRuntime* runtime_ = nullptr;
    JSContext* context_ = nullptr;
    std::unordered_map<std::string,
        std::function<std::any(const std::vector<std::any>&)>>
        registeredFunctions_;
};

}
