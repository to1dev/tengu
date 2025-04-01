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
{
    dataPath_ = QDir::toNativeSeparators(QString("%1/%2/%3")
            .arg(QStandardPaths::writableLocation(
                QStandardPaths::GenericDataLocation))
            .arg(COMPANY)
            .arg(NAME));

    if (!QDir(dataPath_).exists())
        QDir().mkpath(dataPath_);

    // QString path = QDir::toNativeSeparators(QString(argv[0]));
    // mOptions.sysOpt.appPath = mAppPath = QFileInfo(path).absolutePath();

    options_.sysOpt.appPath = appPath_
        = QString::fromStdString(PathUtils::getExecutableDir().string());

    readSettings();

    auto config = std::make_shared<DatabaseConfig>();

    database_ = std::make_unique<Database>(dataPath_, config);

    initLoggins();
}

SettingManager::~SettingManager()
{
    writeSettings();
}

Database* SettingManager::database() const
{
    return database_.get();
}

QString SettingManager::dataPath() const
{
    return dataPath_;
}

QString SettingManager::appPath() const
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
    auto configPath = QDir(dataPath_).filePath("config.toml");
    std::ifstream ifs(configPath.toStdString());
    if (!ifs) {
        std::cerr << "Failed to open config" << std::endl;
        return false;
    }

    try {
        auto tbl = toml::parse(ifs);

        if (auto systemTablePtr
            = tbl.get_as<toml::table>(Settings::STR_SYSTEM_OPTIONS)) {
            const auto& systemTable = *systemTablePtr;
            options_.sysOpt.machineId = QString::fromStdString(
                systemTable["machineId"].value_or<std::string>(""));
        }

        if (auto record
            = tbl.get_as<toml::table>(Settings::STR_RECORD_OPTIONS)) {
            if (auto walletTable
                = record->get_as<toml::table>(Settings::STR_WALLET_OPTIONS)) {
                Wallet& walletOpt = options_.recordOpt.first;
                walletOpt.id = walletTable->contains(Settings::STR_WALLET_ID)
                    ? walletTable->at(Settings::STR_WALLET_ID).value_or<int>(0)
                    : 0;
                walletOpt.type
                    = walletTable->contains(Settings::STR_WALLET_TYPE)
                    ? walletTable->at(Settings::STR_WALLET_TYPE)
                          .value_or<int>(0)
                    : 0;
                walletOpt.groupType
                    = walletTable->contains(Settings::STR_WALLET_GROUPTYPE)
                    ? walletTable->at(Settings::STR_WALLET_GROUPTYPE)
                          .value_or<int>(0)
                    : 0;
                walletOpt.chainType
                    = walletTable->contains(Settings::STR_WALLET_CHAINTYPE)
                    ? walletTable->at(Settings::STR_WALLET_CHAINTYPE)
                          .value_or<int>(0)
                    : 0;
                walletOpt.name
                    = walletTable->contains(Settings::STR_WALLET_NAME)
                    ? walletTable->at("name").value_or<std::string>("")
                    : "";
            }

            if (auto addressTable
                = record->get_as<toml::table>(Settings::STR_ADDRESS_OPTIONS)) {
                Address& addressOpt = options_.recordOpt.second;
                addressOpt.id = addressTable->contains(Settings::STR_ADDRESS_ID)
                    ? addressTable->at(Settings::STR_ADDRESS_ID)
                          .value_or<int>(0)
                    : 0;
                addressOpt.type
                    = addressTable->contains(Settings::STR_ADDRESS_TYPE)
                    ? addressTable->at(Settings::STR_ADDRESS_TYPE)
                          .value_or<int>(0)
                    : 0;
                addressOpt.walletId
                    = addressTable->contains(Settings::STR_ADDRESS_WALLETID)
                    ? addressTable->at("walletId").value_or<int>(0)
                    : 0;
                addressOpt.name
                    = addressTable->contains(Settings::STR_ADDRESS_NAME)
                    ? addressTable->at("name").value_or<std::string>("")
                    : "";
                addressOpt.address
                    = addressTable->contains(Settings::STR_ADDRESS_ADDRESS)
                    ? addressTable->at("address").value_or<std::string>("")
                    : "";
            }
        }
    } catch (const toml::parse_error& err) {
        std::cerr << "Failed to parse config: " << err.description()
                  << std::endl;
        return false;
    }

    return true;
}

bool SettingManager::writeSettings()
{
    auto configPath = QDir(dataPath_).filePath("config.toml");
    toml::table tbl;

    toml::table sysOptTable;
    sysOptTable.insert_or_assign(
        "machineId", options_.sysOpt.machineId.toStdString());
    sysOptTable.insert_or_assign(
        "appPath", options_.sysOpt.appPath.toStdString());
    sysOptTable.insert_or_assign("deviceRatio", options_.sysOpt.deviceRatio);
    sysOptTable.insert_or_assign(
        "dpiSuffix", options_.sysOpt.dpiSuffix.toStdString());
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

    std::ofstream ofs(configPath.toStdString());
    if (!ofs) {
        std::cerr << "Failed to write config" << std::endl;
        return false;
    }
    ofs << tbl;

    return true;
}

const Options& SettingManager::options() const
{
    return options_;
}

void SettingManager::setRecord(Record&& record)
{
    options_.recordOpt = std::move(record);
}

const Record& SettingManager::record_ref() const
{
    return options_.recordOpt;
}

Record SettingManager::record() const
{
    return options_.recordOpt;
}

void SettingManager::initLoggins()
{
    try {
        QString logFilePath
            = QDir::toNativeSeparators(dataPath_ + "/logs/tengu.log");
        QDir logDir(dataPath_ + "/logs");
        if (!logDir.exists()) {
            QDir().mkpath(logDir.absolutePath());
        }

        auto console_sink
            = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logFilePath.toStdString(), 5 * 1024 * 1024, 3);
        file_sink->set_level(spdlog::level::trace);

        spdlog::logger logger("main", { console_sink, file_sink });
        logger.set_level(spdlog::level::debug);

        spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");

        spdlog::info("Logging initialized at: {}", logFilePath.toStdString());
    } catch (const spdlog::spdlog_ex& ex) {
        throw std::runtime_error(
            std::string("Log initialization failed: ") + ex.what());
    }
}
}
