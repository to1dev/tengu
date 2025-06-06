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

#include <any>
#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace Daitengu::Scripting {

class ScriptEngine {
public:
    virtual ~ScriptEngine() = default;

    virtual bool initialize() = 0;
    virtual bool loadScript(std::string_view filePath) = 0;
    virtual bool execute(std::string_view script) = 0;
    virtual std::any callFunction(
        std::string_view funcName, const std::vector<std::any>& args)
        = 0;
    virtual void registerFunction(std::string_view name,
        std::function<std::any(const std::vector<std::any>&)> func)
        = 0;

    template <typename T>
    void registerObject(std::string_view name, T* obj)
    {
        registerObjectImpl(name, obj, std::type_index(typeid(T)));
    }

    virtual void setCallback(std::string_view name,
        std::function<void(const std::vector<std::any>&)> callback)
        = 0;

protected:
    virtual void registerObjectImpl(
        std::string_view name, void* obj, std::type_index type)
        = 0;

    std::unordered_map<std::string_view,
        std::function<void(const std::vector<std::any>&)>>
        callbacks_;
};

}
