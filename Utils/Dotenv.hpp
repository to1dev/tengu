#ifndef DOTENV_H
#define DOTENV_H

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>

#include <windows.h>

#include "ankerl/unordered_dense.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class DotEnvException : public std::runtime_error {
public:
    explicit DotEnvException(const std::string& message)
        : std::runtime_error(message)
    {
    }
};

class DotEnv {
public:
    static inline std::filesystem::path getExePath()
    {
#ifdef _WIN32
        wchar_t path[MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, path, MAX_PATH);
        return std::filesystem::path(path).parent_path();
#else
        return std::filesystem::current_path();
#endif
    }

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

#endif // DOTENV_H
