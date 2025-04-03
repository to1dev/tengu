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

#include "JSScriptEngine.h"

namespace Daitengu::Scripting {

JSScriptEngine::JSScriptEngine()
{
    initialize();
}

JSScriptEngine::~JSScriptEngine()
{
    if (context_)
        JS_FreeContext(context_);

    if (runtime_)
        JS_FreeRuntime(runtime_);
}

bool JSScriptEngine::initialize()
{
    runtime_ = JS_NewRuntime();
    if (!runtime_) {
        spdlog::error("Failed to create QuickJS runtime");
        return false;
    }

    context_ = JS_NewContext(runtime_);
    if (!context_) {
        JS_FreeRuntime(runtime_);
        runtime_ = nullptr;
        spdlog::error("Failed to create QuickJS context");
        return false;
    }

    JS_SetRuntimeInfo(runtime_, "QuickJS Runtime");
    spdlog::info("QuickJS script engine initialized");

    return true;
}

bool JSScriptEngine::loadScript(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        spdlog::error("Failed to open JS script file: {}", filePath);
        return false;
    }

    std::string script((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    file.close();

    return execute(script);
}

bool JSScriptEngine::execute(const std::string& script)
{
    JSValue result = JS_Eval(context_, script.c_str(), script.size(),
        "<script>", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(result)) {
        JSValue exception = JS_GetException(context_);
        const char* error = JS_ToCString(context_, exception);
        spdlog::error(
            "Failed to execute JS script: {}", error ? error : "Unknown error");
        JS_FreeCString(context_, error);
        JS_FreeValue(context_, exception);
        return false;
    }

    JS_FreeValue(context_, result);

    return true;
}

std::any JSScriptEngine::callFunction(
    const std::string& funcName, const std::vector<std::any>& args)
{
    JSValue global = JS_GetGlobalObject(context_);
    JSValue func = JS_GetPropertyStr(context_, global, funcName.c_str());
    if (JS_IsUndefined(func)) {
        spdlog::warn("JS function {} not found", funcName);
        JS_FreeValue(context_, global);
        return std::any {};
    }

    std::vector<JSValue> jsArgs(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i].type() == typeid(int)) {
            jsArgs[i] = JS_NewInt32(context_, std::any_cast<int>(args[i]));
        } else if (args[i].type() == typeid(std::string)) {
            jsArgs[i] = JS_NewString(
                context_, std::any_cast<std::string>(args[i]).c_str());
        } else {
            spdlog::warn(
                "Unsupported argument type for JS function {}", funcName);
            jsArgs[i] = JS_UNDEFINED;
        }
    }

    JSValue result = JS_Call(
        context_, func, global, static_cast<int>(jsArgs.size()), jsArgs.data());
    JS_FreeValue(context_, func);
    JS_FreeValue(context_, global);
    for (auto& arg : jsArgs)
        JS_FreeValue(context_, arg);

    if (JS_IsException(result)) {
        JSValue exception = JS_GetException(context_);
        const char* error = JS_ToCString(context_, exception);
        spdlog::error("Failed to call JS function {}: {}", funcName,
            error ? error : "Unknown error");
        JS_FreeCString(context_, error);
        JS_FreeValue(context_, exception);
        return std::any {};
    }

    std::any retVal;
    if (JS_IsNumber(result)) {
        double val;
        JS_ToFloat64(context_, &val, result);
        retVal = val;
    } else if (JS_IsString(result)) {
        const char* str = JS_ToCString(context_, result);
        retVal = std::string(str ? str : "");
        JS_FreeCString(context_, str);
    }

    JS_FreeValue(context_, result);

    return retVal;
}

void JSScriptEngine::registerFunction(const std::string& name,
    std::function<std::any(const std::vector<std::any>&)> func)
{
    registeredFunctions_[name] = func;
    JSValue global = JS_GetGlobalObject(context_);
    JSValue data = JS_NewInt64(context_, reinterpret_cast<int64_t>(this));
    JSValue jsFunc = JS_NewCFunctionData(context_, jsCallbackWrapper, 0,
        static_cast<int>(registeredFunctions_.size() - 1), 1, &data);
    JS_SetPropertyStr(context_, global, name.c_str(), jsFunc);
    JS_FreeValue(context_, global);
    spdlog::info("Registered C++ function to JS: {}", name);
}

void JSScriptEngine::setCallback(const std::string& name,
    std::function<void(const std::vector<std::any>&)> callback)
{
    callbacks_[name] = callback;
}

void JSScriptEngine::registerObjectImpl(
    const std::string& name, void* obj, std::type_index type)
{
    spdlog::warn("Object registration not yet implemented for JS: {}", name);
}

JSValue JSScriptEngine::jsCallbackWrapper(JSContext* ctx, JSValueConst this_val,
    int argc, JSValueConst* argv, int magic, JSValue* func_data)
{
    JSScriptEngine* engine
        = reinterpret_cast<JSScriptEngine*>(JS_VALUE_GET_PTR(func_data[0]));

    auto it = std::next(engine->registeredFunctions_.begin(), magic);
    std::string name = it->first;

    if (engine->registeredFunctions_.count(name)) {
        std::vector<std::any> args;
        for (int i = 0; i < argc; ++i) {
            if (JS_IsNumber(argv[i])) {
                double val;
                JS_ToFloat64(ctx, &val, argv[i]);
                args.emplace_back(val);
            } else if (JS_IsString(argv[i])) {
                const char* str = JS_ToCString(ctx, argv[i]);
                args.emplace_back(std::string(str ? str : ""));
                JS_FreeCString(ctx, str);
            }
        }
        std::any result = engine->registeredFunctions_[name](args);
        if (result.type() == typeid(double)) {
            return JS_NewFloat64(ctx, std::any_cast<double>(result));
        } else if (result.type() == typeid(std::string)) {
            return JS_NewString(
                ctx, std::any_cast<std::string>(result).c_str());
        }
    }

    return JS_UNDEFINED;
}
}
