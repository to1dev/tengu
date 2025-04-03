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

#include "Splash.h"

namespace Daitengu::Components {

static inline HBITMAP createBitmapFromImage(const QImage& image)
{
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = image.width();
    bmi.bmiHeader.biHeight = -image.height();
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    DCWrapper hdc;
    HBITMAP hBitmap = CreateDIBitmap(
        hdc, &bmi.bmiHeader, CBM_INIT, image.constBits(), &bmi, DIB_RGB_COLORS);

    if (!hBitmap) {
        LOG_ERROR("Failed to create bitmap");
        throw std::runtime_error("Failed to create bitmap");
    }

    return hBitmap;
}

static inline HBITMAP QImageToHBITMAP(
    const QImage& image, bool premultiplied = false)
{
    try {
        if ((premultiplied
                && image.format() == QImage::Format_ARGB32_Premultiplied)
            || (!premultiplied && image.format() == QImage::Format_RGBA8888)) {
            return createBitmapFromImage(image);
        }

        QImage convertedImage = premultiplied
            ? image.convertToFormat(QImage::Format_ARGB32_Premultiplied)
            : image.convertToFormat(QImage::Format_RGBA8888);

        return createBitmapFromImage(convertedImage);
    } catch (const std::exception& e) {
        LOG_ERROR(QString("QImageToHBITMAP failed: %1").arg(e.what()));
        throw;
    }
}

static inline HBITMAP QImageToHBITMAPWithPremultipliedAlpha(const QImage& image)
{
    return QImageToHBITMAP(image, true);
}

SplashImageDownloader::SplashImageDownloader(
    const std::filesystem::path& savePath, const QUrl& url)
    : savePath_(savePath)
    , url_(url)
{
}

SplashImageDownloader::~SplashImageDownloader()
{
    LOG_INFO("Destroy");
}

QCoro::Task<void> SplashImageDownloader::download()
{
    QNetworkRequest request { url_ };
    auto reply = std::unique_ptr<QNetworkReply>(manager_.get(request));
    co_await qCoro(reply.get()).waitForFinished();
    if (reply->isFinished() && reply->error() == QNetworkReply::NoError) {
        QByteArray data = co_await qCoro(reply.get()).readAll();
        QFile file(savePath_.string().c_str());
        if (file.open(QIODevice::WriteOnly)) {
            file.write(data);
            file.close();
            LOG_INFO("Splash image downloaded and saved");
        } else {
            LOG_ERROR("Failed to save splash image");
        }
    } else {
        LOG_ERROR("Failed to download splash image");
    }

    co_return;
}

Splash::Splash(const QPixmap& pixmap, const SplashConfig& config)
    : QSplashScreen(pixmap)
    , config_(config)
{
    LOG_INFO("Initializing Splash Screen");
    try {
        config_.validate();
        std::filesystem::path dataPath
            = PathUtils::getAppDataPath(COMPANY) / NAME;
        QPixmap finalPixmap = pixmap;
        QPixmap localPixmap = loadLocalImage(dataPath);
        if (!localPixmap.isNull()) {
            finalPixmap = localPixmap;
            setPixmap(finalPixmap);
        }
        initializeWindow(finalPixmap);
    } catch (const std::exception& e) {
        LOG_ERROR(
            QString("Failed to initialize splash screen: %1").arg(e.what()));
        recover();
    }
}

void Splash::stayOnTop() const
{
    try {
        if (auto handle = reinterpret_cast<HWND>(winId())) {
            if (!SetWindowPos(handle, HWND_TOPMOST, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE)) {
                LOG_ERROR("Failed to set window position");
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR(QString("stayOnTop failed: %1").arg(e.what()));
    }
}

void Splash::setOpacity(BYTE opacity)
{
    config_.opacity = opacity;
    config_.validate();
    updateLayeredWindow();
}

void Splash::initializeWindow(const QPixmap& pixmap)
{
    auto handle = reinterpret_cast<HWND>(winId());
    if (!handle) {
        LOG_ERROR("Invalid window handle");
        return;
    }

    auto style = GetWindowLong(handle, GWL_EXSTYLE);
    if (!SetWindowLong(handle, GWL_EXSTYLE, style | WS_EX_LAYERED)) {
        LOG_ERROR("Failed to set window style");
        return;
    }

    if (config_.alwaysOnTop) {
        stayOnTop();
    }

    updateLayeredWindow(pixmap);
}

void Splash::updateLayeredWindow(const QPixmap& pixmap)
{
    auto handle = reinterpret_cast<HWND>(winId());
    if (!handle)
        return;

    BLENDFUNCTION bf = { AC_SRC_OVER, 0, config_.opacity, AC_SRC_ALPHA };
    RECT rc;
    GetWindowRect(handle, &rc);

    POINT ptDst = { rc.left, rc.top };
    POINT ptSrc = { 0, 0 };
    SIZE size = { width(), height() };

    try {
        DCWrapper dcScreen;
        std::unique_ptr<HDC__, decltype(&DeleteDC)> dcMem(
            CreateCompatibleDC(dcScreen), DeleteDC);

        if (!dcMem) {
            LOG_ERROR("Failed to create compatible DC");
            return;
        }

        auto hBitmap = QImageToHBITMAPWithPremultipliedAlpha(
            (pixmap.isNull() ? this->pixmap() : pixmap).toImage());

        BitmapSelector selector(dcMem.get(), hBitmap);

        if (!UpdateLayeredWindow(handle, dcScreen, &ptDst, &size, dcMem.get(),
                &ptSrc, 0, &bf, config_.layeredFlags)) {
            LOG_ERROR("UpdateLayeredWindow failed");
        }

        DeleteObject(hBitmap);
    } catch (const std::bad_alloc& e) {
        LOG_ERROR(QString("Memory allocation failed: %1").arg(e.what()));
        recover();
    } catch (const std::runtime_error& e) {
        LOG_ERROR(QString("Runtime error: %1").arg(e.what()));
        recover();
    } catch (const std::exception& e) {
        LOG_ERROR(QString("Unexpected error: %1").arg(e.what()));
        recover();
    }
}

void Splash::recover()
{
    LOG_INFO("Attempting to recover from error...");
    if (auto handle = reinterpret_cast<HWND>(winId())) {
        SetWindowLong(handle, GWL_EXSTYLE, WS_EX_LAYERED);
        ShowWindow(handle, SW_SHOW);
        UpdateWindow(handle);
    }
}

QPixmap Splash::loadLocalImage(const std::filesystem::path& dir)
{
    std::filesystem::path imagePath = dir / "splash.png";
    if (Utils::PathUtils::exists(imagePath)) {
        QPixmap pixmap;
        if (pixmap.load(imagePath.string().c_str())) {
            return pixmap;
        } else {
            LOG_ERROR("[Splash Warning] Local splash image is invalid");
        }
    }
    LOG_INFO("[Splash Info] No valid local splash image found, using default");
    return QPixmap();
}

}
