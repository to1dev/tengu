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

    database_ = std::make_unique<Database>(dataPath_);

    readSettings();
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

bool SettingManager::readSettings()
{
    bool ret = false;

    return ret;
}

bool SettingManager::writeSettings()
{
    bool ret = false;

    return ret;
}

Options& SettingManager::options()
{
    return options_;
}

}
