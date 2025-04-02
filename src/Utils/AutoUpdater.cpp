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

#include "AutoUpdater.h"

namespace Daitengu::Utils {

AutoUpdater::AutoUpdater(QObject* parent)
    : QObject(parent)
    , tempDir_(QDir::tempPath().toStdString() + "/to1dev/tengu/updates")
    , networkManager_(std::make_unique<QNetworkAccessManager>())
{
    fs::create_directories(tempDir_);
}

void AutoUpdater::setGitHubRepo(
    std::string_view username, std::string_view repo)
{
    username_ = username;
    repo_ = repo;
}

void AutoUpdater::setCurrentVersion(std::string_view version)
{
    currentVersion_ = version;
    if (!currentVersion_.empty() && currentVersion_[0] == 'v') {
        currentVersion_.erase(0, 1);
    }
}

void AutoUpdater::setTempDirectory(const fs::path& path)
{
    tempDir_ = path;
    fs::create_directories(tempDir_);
}

void AutoUpdater::setLogCallback(
    std::function<void(Error, const std::string&)> cb)
{
    logCallback_ = std::move(cb);
}

QCoro::Task<std::optional<AutoUpdater::Version>> AutoUpdater::checkForUpdates()
{
    QNetworkRequest request { QUrl(QString::fromStdString(getApiUrl())) };
    request.setHeader(QNetworkRequest::UserAgentHeader, kUserAgent.data());

    NetworkReplyPtr reply { networkManager_->get(request) };
    co_await qCoro(reply.get()).waitForFinished();

    if (reply->error() != QNetworkReply::NoError) {
        auto errorMsg = std::format(
            "Network error: {}", reply->errorString().toStdString());
        logError(Error::NetworkError, errorMsg);
        co_return std::nullopt;
    }

    QByteArray responseData = reply->readAll();
    auto version = parseGitHubResponse(responseData);
    if (!version) {
        co_return std::nullopt;
    }

    auto currentSemver = semver::version { currentVersion_ };
    auto latestSemver = version->asSemver();
    if (!latestSemver) {
        auto errorMsg = "Invalid version format in GitHub response";
        logError(Error::VersionError, errorMsg);
        co_return std::nullopt;
    }

    if (*latestSemver <= currentSemver) {
        Q_EMIT updateCheckComplete(false, *version);
        co_return std::nullopt;
    }

    Q_EMIT updateCheckComplete(true, *version);

    co_return version;
}

QCoro::Task<std::optional<fs::path>> AutoUpdater::downloadUpdate(
    const Version& version,
    std::function<void(const UpdateProgress&)> progressCallback)
{
    std::string filename = std::format("update_{}_{}", version.version,
        fs::path(version.downloadUrl).filename().string());

    fs::path filePath = tempDir_ / filename;

    QNetworkRequest request { QUrl(
        QString::fromStdString(version.downloadUrl)) };
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
        QNetworkRequest::NoLessSafeRedirectPolicy);

    std::unique_ptr<QFile> downloadFile
        = std::make_unique<QFile>(QString::fromStdString(filePath.string()));

    if (!downloadFile->open(QIODevice::WriteOnly)) {
        auto errorMsg = std::format(
            "Cannot open file for writing: {}", filePath.string());
        logError(Error::DownloadError, errorMsg);
        co_return std::nullopt;
    }

    NetworkReplyPtr reply { networkManager_->get(request) };
    QObject::connect(reply.get(), &QNetworkReply::downloadProgress,
        [this, progressCallback](qint64 bytesReceived, qint64 bytesTotal) {
            Q_EMIT downloadProgress(bytesReceived, bytesTotal);

            if (progressCallback) {
                UpdateProgress progress { static_cast<uint64_t>(bytesReceived),
                    static_cast<uint64_t>(bytesTotal) };
                progressCallback(progress);
            }
        });

    co_await qCoro(reply.get()).waitForFinished();

    if (reply->bytesAvailable() > 0) {
        downloadFile->write(reply->readAll());
    }
    downloadFile->close();

    if (reply->error() != QNetworkReply::NoError) {
        auto errorMsg = std::format(
            "Download error: {}", reply->errorString().toStdString());
        logError(Error::DownloadError, errorMsg);
        if (fs::exists(filePath))
            fs::remove(filePath);

        co_return std::nullopt;
    }

    Q_EMIT downloadComplete(QString::fromStdString(filePath.string()));

    co_return filePath;
}

class WindowsInstallStrategy : public InstallStrategy {
public:
    bool install(const fs::path& updateFile) override
    {
        std::string ext = updateFile.extension().string();
        if (ext == ".exe" || ext == ".msi") {
            QProcess::startDetached(
                QString::fromStdString(updateFile.string()), {});
            QCoreApplication::quit();
            return true;
        }
        QDesktopServices::openUrl(
            QUrl::fromLocalFile(QString::fromStdString(updateFile.string())));
        return true;
    }
};

class MacOSInstallStrategy : public InstallStrategy {
public:
    bool install(const fs::path& updateFile) override
    {
        std::string ext = updateFile.extension().string();
        if (ext == ".dmg" || ext == ".pkg") {
            QDesktopServices::openUrl(QUrl::fromLocalFile(
                QString::fromStdString(updateFile.string())));
            QCoreApplication::quit();
            return true;
        }
        QDesktopServices::openUrl(
            QUrl::fromLocalFile(QString::fromStdString(updateFile.string())));
        return true;
    }
};

class LinuxInstallStrategy : public InstallStrategy {
public:
    bool install(const fs::path& updateFile) override
    {
        std::string ext = updateFile.extension().string();
        if (ext == ".AppImage") {
            fs::permissions(updateFile,
                fs::perms::owner_exec | fs::perms::group_exec
                    | fs::perms::others_exec,
                fs::perm_options::add);
            QProcess::startDetached(
                QString::fromStdString(updateFile.string()), {});
            QCoreApplication::quit();
            return true;
        } else if (ext == ".deb" || ext == ".rpm") {
            QDesktopServices::openUrl(QUrl::fromLocalFile(
                QString::fromStdString(updateFile.string())));
            return true;
        }
        QDesktopServices::openUrl(
            QUrl::fromLocalFile(QString::fromStdString(updateFile.string())));
        return true;
    }
};

bool AutoUpdater::installUpdate(const fs::path& updateFile)
{
    if (!fs::exists(updateFile)) {
        auto errorMsg = std::format(
            "Update file does not exist: {}", updateFile.string());
        logError(Error::InstallationError, errorMsg);
        return false;
    }

    auto strategy = createInstallStrategy();
    if (strategy) {
        return strategy->install(updateFile);
    }

    QDesktopServices::openUrl(
        QUrl::fromLocalFile(QString::fromStdString(updateFile.string())));
    return true;
}

std::string AutoUpdater::getApiUrl() const
{
    return std::format("{}", kApiBaseUrl, username_, repo_);
}

std::optional<AutoUpdater::Version> AutoUpdater::parseGitHubResponse(
    const QByteArray& response)
{
    try {
        json releaseJson = json::parse(response.constData());

        std::string tagName = releaseJson["tag_name"].get<std::string>();
        if (!tagName.empty() && tagName[0] == 'v') {
            tagName.erase(0, 1);
        }

        std::string releaseNotes = releaseJson["body"].get<std::string>();

        QString platformAssetPattern = getPlatformSpecificAssetName();
        std::string downloadUrl;
        uint64_t fileSize = 0;

        QRegularExpression regex { platformAssetPattern };

        for (const auto& asset : releaseJson["assets"]) {
            std::string assetName = asset["name"].get<std::string>();
            if (regex.match(QString::fromStdString(assetName)).hasMatch()) {
                downloadUrl = asset["browser_download_url"].get<std::string>();
                fileSize = asset["size"].get<uint64_t>();
                break;
            }
        }

        if (downloadUrl.empty()) {
            auto errorMsg = "No suitable download found for this platform";
            logError(Error::ParsingError, errorMsg);
            return std::nullopt;
        }

        return Version {
            tagName,
            downloadUrl,
            releaseNotes,
            fileSize,
        };
    } catch (const json::exception& e) {
        auto errorMsg = std::format("JSON parsing error: {}", e.what());
        logError(Error::ParsingError, errorMsg);
        return std::nullopt;
    }
}

QString AutoUpdater::getPlatformSpecificAssetName()
{
#if defined(Q_OS_WIN)
    return ".*\\.(exe|msi|zip)$";
#elif defined(Q_OS_MACOS)
    return ".*\\.(dmg|pkg|zip)$";
#elif defined(Q_OS_LINUX)
    return ".*\\.(AppImage|deb|rpm|tar\\.gz|tar\\.xz)$";
#else
    return ".*";
#endif
}

void AutoUpdater::logError(Error error, const std::string& msg)
{
    Q_EMIT errorOccurred(static_cast<int>(error), QString::fromStdString(msg));
    if (logCallback_)
        logCallback_(error, msg);
}

std::unique_ptr<InstallStrategy> AutoUpdater::createInstallStrategy() const
{
#if defined(Q_OS_WIN)
    return std::make_unique<WindowsInstallStrategy>();
#elif defined(Q_OS_MACOS)
    return std::make_unique<MacOSInstallStrategy>();
#elif defined(Q_OS_LINUX)
    return std::make_unique<LinuxInstallStrategy>();
#else
    return nullptr; // Fallback to default behavior
#endif
}
}
