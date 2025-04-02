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

#include "SettingManager.h"

namespace Daitengu::Core {

SettingManager::SettingManager()
    : dataPath_(PathUtils::getAppDataPath(COMPANY) / NAME)
    , appPath_(PathUtils::getExecutableDir())
{
    if (!fs::exists(dataPath_)) {
        if (!PathUtils::createDirectories(dataPath_)) {
            throw std::runtime_error(
                "Failed to create data directory: " + dataPath_.string());
        }
    }

    // QString path = QDir::toNativeSeparators(QString(argv[0]));
    // mOptions.sysOpt.appPath = mAppPath = QFileInfo(path).absolutePath();

    options_.sysOpt.appPath = appPath_.string();

    initLogging();

    if (!readSettings()) {
        spdlog::warn("Failed to read settings, using defaults.");
    }

    database_
        = std::make_unique<Database>(QString::fromStdString(dataPath_.string()),
            std::make_shared<DatabaseConfig>());
}

SettingManager::~SettingManager()
{
    writeSettings();
}

Database* SettingManager::database() const noexcept
{
    return database_.get();
}

const fs::path& SettingManager::dataPath() const noexcept
{
    return dataPath_;
}

const fs::path& SettingManager::appPath() const noexcept
{
    return appPath_;
}

/*
void modifyTomlExample()
{
    try {
        auto tbl = toml::parse_file("example.toml");

        tbl["version"] = 1.1;
        tbl["active"] = false;

        tbl.insert_or_assign("updated", true);

        if (auto server = tbl["server"].as_table()) {
            (*server)["port"] = 9090;
        }

        if (auto numbers = tbl["numbers"].as_array()) {
            numbers->push_back(4);
            numbers->push_back(5);
        }

        if (auto users = tbl["users"].as_array()) {
            toml::table user3;
            user3.insert("name", "Charlie");
            user3.insert("role", "moderator");
            users->push_back(user3);
        }

        std::ofstream file("example_modified.toml");
        file << tbl;
        file.close();

    } catch (const toml::parse_error& err) {
        std::cerr << "Error parsing file: " << err.description() << std::endl;
    }
}

void safeAccessExample()
{
    try {
        auto tbl = toml::parse_file("example.toml");

        int count = tbl["count"].value_or(0);

        if (tbl.contains("database")) {
            if (tbl["database"].is_table()) {
                auto& db = *tbl["database"].as_table();
            }
        }

        if (auto opt_str = tbl["title"].value<std::string>()) {
            std::string title = *opt_str;
            std::cout << "Title: " << title << std::endl;
        }

        for (auto&& [key, value] : tbl) {
            std::cout << "Key: " << key << ", Type: ";
            if (value.is_string())
                std::cout << "string" << std::endl;
            else if (value.is_integer())
                std::cout << "integer" << std::endl;
            else if (value.is_floating_point())
                std::cout << "float" << std::endl;
            else if (value.is_boolean())
                std::cout << "boolean" << std::endl;
            else if (value.is_array())
                std::cout << "array" << std::endl;
            else if (value.is_table())
                std::cout << "table" << std::endl;
            else
                std::cout << "other" << std::endl;
        }

    } catch (const toml::parse_error& err) {
        std::cerr << "Parse error: " << err.description() << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
}
*/

bool SettingManager::readSettings()
{
    auto configPath = dataPath_ / "config.toml";
    std::ifstream ifs(configPath);
    if (!ifs.is_open()) {
        return false;
    }

    try {
        auto tbl = toml::parse(ifs);

        if (auto systemTable = tbl[Settings::STR_SYSTEM_OPTIONS].as_table()) {
            options_.sysOpt.machineId
                = systemTable->at("machineId").value_or<std::string>("");
        }

        if (auto record = tbl[Settings::STR_RECORD_OPTIONS].as_table()) {
            if (auto walletTable
                = record->at(Settings::STR_WALLET_OPTIONS).as_table()) {
                auto& walletOpt = options_.recordOpt.first;
                walletOpt.id
                    = walletTable->at(Settings::STR_WALLET_ID).value_or(0);
                walletOpt.type
                    = walletTable->at(Settings::STR_WALLET_TYPE).value_or(0);
                walletOpt.groupType
                    = walletTable->at(Settings::STR_WALLET_GROUPTYPE)
                          .value_or(0);
                walletOpt.chainType
                    = walletTable->at(Settings::STR_WALLET_CHAINTYPE)
                          .value_or(0);
                walletOpt.name = walletTable->at(Settings::STR_WALLET_NAME)
                                     .value_or<std::string>("");
            }

            if (auto addressTable
                = record->at(Settings::STR_ADDRESS_OPTIONS).as_table()) {
                auto& addressOpt = options_.recordOpt.second;
                addressOpt.id
                    = addressTable->at(Settings::STR_ADDRESS_ID).value_or(0);
                addressOpt.type
                    = addressTable->at(Settings::STR_ADDRESS_TYPE).value_or(0);
                addressOpt.walletId
                    = addressTable->at(Settings::STR_ADDRESS_WALLETID)
                          .value_or(0);
                addressOpt.name = addressTable->at(Settings::STR_ADDRESS_NAME)
                                      .value_or<std::string>("");
                addressOpt.address
                    = addressTable->at(Settings::STR_ADDRESS_ADDRESS)
                          .value_or<std::string>("");
            }
        }
    } catch (const toml::parse_error& err) {
        spdlog::error("Failed to parse config: {}", err.description());
        return false;
    }

    return true;
}

bool SettingManager::writeSettings()
{
    auto configPath = dataPath_ / "config.toml";
    toml::table tbl;

    toml::table sysOptTable;
    sysOptTable.insert_or_assign("machineId", options_.sysOpt.machineId);
    sysOptTable.insert_or_assign("appPath", options_.sysOpt.appPath);
    sysOptTable.insert_or_assign("deviceRatio", options_.sysOpt.deviceRatio);
    sysOptTable.insert_or_assign("dpiSuffix", options_.sysOpt.dpiSuffix);
    tbl.insert_or_assign(Settings::STR_SYSTEM_OPTIONS, sysOptTable);

    toml::table walletTable;
    walletTable.insert_or_assign("id", options_.recordOpt.first.id);
    walletTable.insert_or_assign("type", options_.recordOpt.first.type);
    walletTable.insert_or_assign(
        "groupType", options_.recordOpt.first.groupType);
    walletTable.insert_or_assign(
        "chainType", options_.recordOpt.first.chainType);
    walletTable.insert_or_assign("name", options_.recordOpt.first.name);

    toml::table addressTable;
    addressTable.insert_or_assign("id", options_.recordOpt.second.id);
    addressTable.insert_or_assign("type", options_.recordOpt.second.type);
    addressTable.insert_or_assign(
        "walletId", options_.recordOpt.second.walletId);
    addressTable.insert_or_assign("name", options_.recordOpt.second.name);
    addressTable.insert_or_assign("address", options_.recordOpt.second.address);

    toml::table recordTable;
    recordTable.insert_or_assign(Settings::STR_WALLET_OPTIONS, walletTable);
    recordTable.insert_or_assign(Settings::STR_ADDRESS_OPTIONS, addressTable);
    tbl.insert_or_assign(Settings::STR_RECORD_OPTIONS, recordTable);

    std::ofstream ofs(configPath);
    if (!ofs.is_open()) {
        spdlog::error("Failed to write config to: {}", configPath.string());
        return false;
    }
    ofs << tbl;

    return true;

    /*
    toml::array arr;
    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);
    tbl.insert("numbers", arr);

    tbl.insert("created", toml::date_time({2023, 4, 12}, {15, 30, 0}));

    toml::table serverTable;
    serverTable.insert("host", "localhost");
    serverTable.insert("port", 8080);
    tbl.insert("server", serverTable);

    toml::array usersArray;

    toml::table user1;
    user1.insert("name", "Alice");
    user1.insert("role", "admin");
    usersArray.push_back(user1);

    toml::table user2;
    user2.insert("name", "Bob");
    user2.insert("role", "user");
    usersArray.push_back(user2);

    tbl.insert("users", usersArray);
    */
}

const Options& SettingManager::options() const noexcept
{
    return options_;
}

void SettingManager::setRecord(Record&& record) noexcept
{
    options_.recordOpt = std::move(record);
}

const Record& SettingManager::record_ref() const noexcept
{
    return options_.recordOpt;
}

Record SettingManager::record() const noexcept
{
    return options_.recordOpt;
}

void SettingManager::initLogging()
{
    try {
        auto logDir = dataPath_ / "logs";
        auto logFilePath = logDir / "tengu.log";
        if (!fs::exists(logDir)) {
            if (!PathUtils::createDirectories(logDir)) {
                throw std::runtime_error(
                    "Failed to create log directory: " + logDir.string());
            }
        }

        auto console_sink
            = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logFilePath.string(), 5 * 1024 * 1024, 3);
        file_sink->set_level(spdlog::level::trace);

        std::vector<spdlog::sink_ptr> sinks { console_sink, file_sink };
        auto logger = std::make_shared<spdlog::logger>(
            "main", sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::debug);
        spdlog::set_default_logger(logger);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");

        spdlog::info("Logging initialized at: {}", logFilePath.string());
    } catch (const spdlog::spdlog_ex& ex) {
        throw std::runtime_error(
            std::string("Log initialization failed: ") + ex.what());
    }
}
}
