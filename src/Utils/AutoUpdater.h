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

#include <expected>
#include <filesystem>
#include <format>
#include <functional>
#include <optional>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QProcess>
#include <QRegularExpression>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

#include <qcoro/QCoroNetworkReply>
#include <qcoro/QCoroTask>

#include "semver.hpp"

namespace Daitengu::Utils {

class AutoUpdater : public QObject {
    Q_OBJECT

public:
    struct Version {
        std::string version;
        std::string downloadUrl;
        std::string releaseNotes;
        uint64_t fileSize { 0 };

        [[nodiscard]] std::optional<semver::version> asSemver() const noexcept
        {
            try {
                return semver::version { version };
            } catch (...) {
                return std::nullopt;
            }
        }
    };

    struct UpdateProgress {
        uint64_t bytesReceived { 0 };
        uint64_t bytesTotal { 0 };

        [[nodiscard]] float percentage() const noexcept
        {
            return bytesTotal > 0
                ? static_cast<float>(bytesReceived) / bytesTotal * 100.0f
                : 0.0f;
        }
    };

    enum class Error {
        NetworkError,
        ParsingError,
        DownloadError,
        InstallationError,
        VersionError
    };

public:
    explicit AutoUpdater(QObject* parent = nullptr);
    ~AutoUpdater() override;

    void setGitHubRepo(std::string_view username, std::string_view repo);
    void setCurrentVersion(std::string_view version);
    void setTempDirectory(const fs::path& path);

    QCoro::Task<std::optional<Version>> checkForUpdates();

    QCoro::Task<std::optional<fs::path>> downloadUpdate(const Version& version,
        std::function<void(const UpdateProgress&)> progressCallback = {});

    bool installUpdate(const fs::path& updateFile);

Q_SIGNALS:
    void updateCheckComplete(bool updateAvailable, const Version& version);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadComplete(const QString& filePath);
    void errorOccurred(int errorCode, const QString& errorMessage);

private:
    std::string username_;
    std::string repo_;
    std::string currentVersion_;
    fs::path tempDir_;

    std::unique_ptr<QNetworkAccessManager> networkManager_;
    std::unique_ptr<QFile> downloadFile_;

    [[nodiscard]] std::string getApiUrl() const;
    std::optional<Version> parseGitHubResponse(const QByteArray& response);
    static QString getPlatformSpecificAssetName();
};
}
