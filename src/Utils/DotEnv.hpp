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

#ifndef DOTENV_H
#define DOTENV_H

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#endif

#include "ankerl/unordered_dense.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace Daitengu::Utils {

class DotEnvException : public std::runtime_error {
public:
    explicit DotEnvException(const std::string& message)
        : std::runtime_error(message)
    {
    }
};

class DotEnv {
public:
    static DotEnv& getInstance()
    {
        static DotEnv instance;
        return instance;
    }

    DotEnv(const DotEnv&) = delete;
    DotEnv& operator=(const DotEnv&) = delete;

    void load(const std::string& envFile = "", const std::string& jsonFile = "")
    {
        try {
            if (!envFile.empty())
                loadEnvFile(envFile);

            if (!jsonFile.empty())
                loadJsonFile(jsonFile);
        } catch (const std::exception& e) {
            throw DotEnvException(
                "Failed to load configuration: " + std::string(e.what()));
        }
    }

    std::optional<std::string> get(const std::string& key) const
    {
        auto it = env.find(key);
        return it != env.end() ? std::optional<std::string>(it->second)
                               : std::nullopt;
    }

    std::string getOrDefault(
        const std::string& key, const std::string& defaultValue = "")
    {
        return get(key).value_or(defaultValue);
    }

    bool has(const std::string& key) const
    {
        return env.find(key) != env.end();
    }

    void clear()
    {
        env.clear();
    }

private:
    DotEnv() = default;
    ankerl::unordered_dense::map<std::string, std::string> env;

    void loadEnvFile(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file) {
            throw DotEnvException("Cannot open env file: " + filename);
        }

        std::string line;
        size_t lineNumber = 0;
        while (std::getline(file, line)) {
            ++lineNumber;
            if (shouldSkipLine(line))
                continue;

            try {
                parseLine(line);
            } catch (const std::exception& e) {
                throw DotEnvException("Error parsing line "
                    + std::to_string(lineNumber) + ": " + e.what());
            }
        }
    }

    void loadJsonFile(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file) {
            throw DotEnvException("Cannot open JSON file: " + filename);
        }

        try {
            json j;
            file >> j;
            parseJson(j);
        } catch (const json::exception& e) {
            throw DotEnvException("JSON parse error: " + std::string(e.what()));
        }
    }

    bool shouldSkipLine(const std::string& line) const
    {
        return line.empty() || line[0] == '#'
            || line.find_first_not_of(" \t") == std::string::npos;
    }

    void parseLine(const std::string& line)
    {
        size_t pos = line.find('=');
        if (pos == std::string::npos) {
            throw DotEnvException("Invalid format: missing '=' separator");
        }

        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));

        if (key.empty()) {
            throw DotEnvException("Empty key is not allowed");
        }

        env[key] = value;
    }

    void parseJson(const json& j)
    {
        for (const auto& [key, value] : j.items()) {
            env[key]
                = value.is_string() ? value.get<std::string>() : value.dump();
        }
    }

    static std::string trim(const std::string& str)
    {
        const std::string whitespace = " \t\r\n";
        const size_t first = str.find_first_not_of(whitespace);

        if (first == std::string::npos)
            return "";

        const size_t last = str.find_last_not_of(whitespace);

        return str.substr(first, last - first + 1);
    }
};

}
#endif // DOTENV_H
