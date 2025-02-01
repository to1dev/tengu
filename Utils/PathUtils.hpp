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
