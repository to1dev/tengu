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

#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "toml.hpp"

#include "Consts.h"
#include "Globals.h"

#include "Databases/Database.h"
#include "Utils/PathUtils.hpp"

using namespace Daitengu::Core;
using namespace Daitengu::Databases;
using namespace Daitengu::Utils;

namespace Daitengu::Core {

namespace Settings {
    inline constexpr std::string_view STR_SYSTEM_OPTIONS = "System";
    inline constexpr std::string_view STR_RECORD_OPTIONS = "Record";
    inline constexpr std::string_view STR_WALLET_OPTIONS = "Wallet";
    inline constexpr std::string_view STR_ADDRESS_OPTIONS = "Address";
    inline constexpr std::string_view STR_WALLET_ID = "id";
    inline constexpr std::string_view STR_WALLET_TYPE = "type";
    inline constexpr std::string_view STR_WALLET_GROUPTYPE = "groupType";
    inline constexpr std::string_view STR_WALLET_CHAINTYPE = "chainType";
    inline constexpr std::string_view STR_WALLET_NAME = "name";
    inline constexpr std::string_view STR_ADDRESS_ID = "id";
    inline constexpr std::string_view STR_ADDRESS_TYPE = "type";
    inline constexpr std::string_view STR_ADDRESS_WALLETID = "walletId";
    inline constexpr std::string_view STR_ADDRESS_NAME = "name";
    inline constexpr std::string_view STR_ADDRESS_ADDRESS = "address";
}

struct SystemOptions {
    std::string machineId;
    std::string appPath;
    double deviceRatio {};
    std::string dpiSuffix;
};

struct Options {
    SystemOptions sysOpt;
    Record recordOpt;
};

class SettingManager {
public:
    explicit SettingManager();
    ~SettingManager();

    [[nodiscard]] Database* database() const noexcept;

    [[nodiscard]] const fs::path& dataPath() const noexcept;
    [[nodiscard]] const fs::path& appPath() const noexcept;

    [[nodiscard]] const Options& options() const noexcept;

    void setRecord(Record&& record) noexcept;
    [[nodiscard]] const Record& record_ref() const noexcept;
    [[nodiscard]] Record record() const noexcept;

private:
    void initLogging();
    bool readSettings();
    bool writeSettings();

private:
    fs::path dataPath_;
    fs::path appPath_;
    Options options_ {};

    std::unique_ptr<Database> database_ { nullptr };
};

}
