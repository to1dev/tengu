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

#ifndef SPLASH_H
#define SPLASH_H

#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>
#include <QSplashScreen>
#include <QUrl>

#ifdef WIN32
#include <windows.h>
#endif

#include <spdlog/spdlog.h>

#include <qcoro/QCoro>

#include "Consts.h"

#include "Utils/PathUtils.hpp"

using namespace Daitengu::Core;
using namespace Daitengu::Utils;

namespace Daitengu::Components {

#define LOG_ERROR(msg) spdlog::error("[Splash Error] {}", msg);
#define LOG_INFO(msg) spdlog::info("[Splash Info] {}", msg);

struct SplashConfig {
    bool download = true;
    bool alwaysOnTop = true;
    BYTE opacity = 0xFF;
    DWORD layeredFlags = ULW_ALPHA;

    void validate() const
    {
        if (opacity > 255) {
            throw std::invalid_argument("Opacity must be 0-255");
        }
    }
};

class DCWrapper {
public:
    explicit DCWrapper(HWND hwnd = nullptr) noexcept(false)
        : hwnd_(hwnd)
        , dc_(GetDC(hwnd))
    {
        if (!dc_) {
            LOG_ERROR("Failed to get DC");
            throw std::runtime_error("Failed to get DC");
        }
    }

    DCWrapper(const DCWrapper&) = delete;
    DCWrapper& operator=(const DCWrapper&) = delete;

    ~DCWrapper() noexcept
    {
        if (dc_)
            ReleaseDC(hwnd_, dc_);
    }

    operator HDC() const
    {
        return dc_;
    }

private:
    HDC dc_;
    HWND hwnd_;
};

class BitmapSelector {
public:
    BitmapSelector(HDC hdc, HBITMAP bitmap)
        : dc_(hdc)
    {
        if (!bitmap) {
            LOG_ERROR("Invalid bitmap handle");
            throw std::runtime_error("Invalid bitmap handle");
        }
        oldBitmap_ = static_cast<HBITMAP>(SelectObject(dc_, bitmap));
    }

    ~BitmapSelector()
    {
        if (oldBitmap_)
            SelectObject(dc_, oldBitmap_);
    }

private:
    HDC dc_;
    HBITMAP oldBitmap_;
};

class SplashImageDownloader {
public:
    explicit SplashImageDownloader(const std::filesystem::path& savePath,
        const QUrl& url = QUrl("https://img.tengu.to1.dev/splash.png"));
    ~SplashImageDownloader();

    QCoro::Task<void> download();

private:
    QNetworkAccessManager manager_;
    std::filesystem::path savePath_;
    QUrl url_;
};

class Splash : public QSplashScreen {
public:
    explicit Splash(
        const QPixmap& pixmap, const SplashConfig& config = SplashConfig());

    void stayOnTop() const;
    void setOpacity(BYTE opacity);

private:
    SplashConfig config_;

    void initializeWindow(const QPixmap& pixmap);
    void updateLayeredWindow(const QPixmap& pixmap = QPixmap());
    void recover();

    QPixmap loadLocalImage(const std::filesystem::path& dir);
};

}
#endif // SPLASH_H
