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

AutoUpdater::~AutoUpdater() = default;

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

QCoro::Task<std::optional<AutoUpdater::Version>> AutoUpdater::checkForUpdates()
{
    QNetworkRequest request { QUrl(QString::fromStdString(getApiUrl())) };
    request.setHeader(
        QNetworkRequest::UserAgentHeader, "Tengu-AutoUpdater/1.0");

    QNetworkReply* reply = networkManager_->get(request);
    try {
        co_await qCoro(reply).waitForFinished();

        if (reply->error() != QNetworkReply::NoError) {
            Q_EMIT errorOccurred(static_cast<int>(Error::NetworkError),
                QString::fromStdString(std::format(
                    "Network error: {}", reply->errorString().toStdString())));
            reply->deleteLater();

            co_return std::nullopt;
        }

        QByteArray responseData = reply->readAll();
        reply->deleteLater();

        auto version = parseGitHubResponse(responseData);
        if (!version) {
            co_return std::nullopt;
        }

        auto currentSemver = semver::version { currentVersion_ };
        auto latestSemver = version->asSemver();

        if (!latestSemver) {
            Q_EMIT errorOccurred(static_cast<int>(Error::VersionError),
                "Invalid version format in GitHub response");
            co_return std::nullopt;
        }

        if (*latestSemver <= currentSemver) {
            Q_EMIT updateCheckComplete(false, *version);
            co_return std::nullopt;
        }

        Q_EMIT updateCheckComplete(true, *version);
        co_return version;
    } catch (const std::exception& e) {
        Q_EMIT errorOccurred(static_cast<int>(Error::NetworkError),
            QString::fromStdString(std::format("Exception: {}", e.what())));
        reply->deleteLater();
    }

    co_return std::nullopt;
}

QCoro::Task<std::optional<fs::path>> AutoUpdater::downloadUpdate(
    const Version& version,
    std::function<void(const UpdateProgress&)> progressCallback)
{
    return QCoro::Task<std::optional<fs::path>>();
}

bool AutoUpdater::installUpdate(const fs::path& updateFile)
{
    return false;
}

std::string AutoUpdater::getApiUrl() const
{
    return std::format(
        "https://api.github.com/repos/{}/{}/releases/latest", username_, repo_);
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
            Q_EMIT errorOccurred(static_cast<int>(Error::ParsingError),
                "No suitable download found for this platform");
            return std::nullopt;
        }

        return Version {
            tagName,
            downloadUrl,
            releaseNotes,
            fileSize,
        };
    } catch (const json::exception& e) {
        Q_EMIT errorOccurred(static_cast<int>(Error::ParsingError),
            QString::fromStdString(
                std::format("JSON parsing error: {}", e.what())));
    }

    return std::nullopt;
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
}
