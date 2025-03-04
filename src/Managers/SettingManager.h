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

#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include <QDir>
#include <QStandardPaths>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

#include "Consts.h"
#include "Globals.h"

#include "Databases/Database.h"
#include "Utils/PathUtils.hpp"

using namespace Daitengu::Core;
using namespace Daitengu::Databases;
using namespace Daitengu::Utils;

namespace Daitengu::Core {

inline constexpr char STR_WALLET_OPTIONS[] = "Wallet";
inline constexpr char STR_WALLET_ID[] = "walletId";
inline constexpr char STR_WALLET_TYPE[] = "walletType";
inline constexpr char STR_VAULT_OPTIONS[] = "Vault";
inline constexpr char STR_VAULT_ID[] = "vaultId";
inline constexpr char STR_VAULT_TYPE[] = "vaultType";
inline constexpr char STR_ADDRESS_ID[] = "id";
inline constexpr char STR_ADDRESS_TYPE[] = "type";

struct SystemOptions {
    QString machineId;
    QString appPath;
    double deviceRatio;
    QString dpiSuffix;
};
struct Options {
    SystemOptions sysOpt;
};

class SettingManager {
public:
    SettingManager();
    ~SettingManager();

    Database* database() const;

    QString dataPath() const;
    QString appPath() const;

    bool readSettings();
    bool writeSettings();

    Options& options();

private:
    QString dataPath_;
    QString appPath_;
    Options options_;

    std::unique_ptr<Database> database_;
};

}
#endif // SETTINGMANAGER_H
