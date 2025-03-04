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

#ifndef PATHUTILS_HPP
#define PATHUTILS_HPP

#include <filesystem>

#ifdef _WIN32
#include <shlobj.h>
#include <windows.h>
#elif defined(__linux__)
#elif defined(__APPLE__)
#endif

namespace Daitengu::Utils {

class PathUtils {
public:
    static std::filesystem::path getExecutablePath()
    {
#ifdef _WIN32
        wchar_t path[MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, path, MAX_PATH);
        return std::filesystem::path(path);
#elif defined(__linux__)
        // Todo
#elif defined(__APPLE__)
        // Todo
#else
        return std::filesystem::current_path();
#endif
    }

    static std::filesystem::path getExecutableDir()
    {
        return getExecutablePath().parent_path();
    }

    static std::filesystem::path getCurrentPath()
    {
        return std::filesystem::current_path();
    }

    static std::filesystem::path getTempPath()
    {
        return std::filesystem::temp_directory_path();
    }

    static std::filesystem::path getHomePath()
    {
#ifdef _WIN32
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path))) {
            return std::filesystem::path(path);
        }
#else
        // Todo
#endif
        return {};
    }

    static std::filesystem::path getAppDataPath(const std::string& appName)
    {
#ifdef _WIN32
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, path))) {
            return std::filesystem::path(path) / appName;
        }
#elif defined(__APPLE__)
        // Todo
#else
        return getHomePath() / ("." + appName);
#endif
        return {};
    }

    static std::filesystem::path toAbsolutePath(
        const std::filesystem::path& path)
    {
        if (path.is_absolute()) {
            return path;
        }
        return std::filesystem::absolute(path);
    }

    static std::filesystem::path normalizePath(
        const std::filesystem::path& path)
    {
        return std::filesystem::weakly_canonical(path);
    }

    static std::string getExtension(const std::filesystem::path& path)
    {
        return path.extension().string();
    }

    static bool exists(const std::filesystem::path& path)
    {
        return std::filesystem::exists(path);
    }

    static bool createDirectories(const std::filesystem::path& path)
    {
        try {
            return std::filesystem::create_directories(path);
        } catch (...) {
            return false;
        }
    }
};

}

#endif // PATHUTILS_HPP
