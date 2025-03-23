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

        options_.sysOpt.machineId = QString::fromStdString(
            tbl[Settings::STR_SYSTEM_OPTIONS]["machineId"]
                .value_or<std::string>(""));
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
    walletTable.insert_or_assign("name", options_.recordOpt.first.name);
    tbl.insert_or_assign(Settings::STR_WALLET_OPTIONS, walletTable);

    toml::table addressTable;
    addressTable.insert_or_assign("id", options_.recordOpt.second.id);
    addressTable.insert_or_assign("type", options_.recordOpt.second.type);
    addressTable.insert_or_assign(
        "walletId", options_.recordOpt.second.walletId);
    addressTable.insert_or_assign("name", options_.recordOpt.second.name);
    addressTable.insert_or_assign("address", options_.recordOpt.second.address);
    tbl.insert_or_assign(Settings::STR_ADDRESS_OPTIONS, addressTable);

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
}
