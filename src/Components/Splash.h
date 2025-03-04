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

#include <memory>
#include <stdexcept>

#include <QDebug>
#include <QMutex>
#include <QSplashScreen>
#include <QThread>
#include <QWaitCondition>

#include <windows.h>

namespace Daitengu::Components {

inline constexpr int SLEEP_TIME = 1;

#define LOG_ERROR(msg) qCritical() << "[Splash Error]" << msg
#define LOG_INFO(msg) qInfo() << "[Splash Info]" << msg

class SplashThread : public QThread {
    Q_OBJECT

public:
    explicit SplashThread(QObject* parent = nullptr);

    static void sleep(unsigned long secs);
    static void msleep(unsigned long msecs);
    static void usleep(unsigned long usecs);

    bool interruptibleSleep(unsigned long secs);

protected:
    void run() override;

Q_SIGNALS:
    void started();
    void finished();
    void error(const QString& errorMessage);

public Q_SLOTS:
    void requestStop();
};

class SplashThreadEx : public SplashThread {
    Q_OBJECT

public:
    explicit SplashThreadEx(QObject* parent = nullptr);

    void pause();
    void resume();

    bool interruptibleSleepWithTimeout(
        unsigned long secs, unsigned long timeoutSecs);

protected:
    void checkPause();

private:
    QMutex mutex_;
    QWaitCondition condition_;
    bool isPaused_;
};

struct SplashConfig {
    bool alwaysOnTop = true;
    BYTE opacity = 0xFF;
    DWORD layeredFlags = ULW_ALPHA;
};

class DCWrapper {
public:
    DCWrapper(HWND h = NULL)
        : hwnd(h)
    {
        dc = GetDC(hwnd);
        if (!dc) {
            LOG_ERROR("Failed to get DC");
            throw std::runtime_error("Failed to get DC");
        }
    }

    ~DCWrapper()
    {
        if (dc)
            ReleaseDC(hwnd, dc);
    }

    operator HDC() const
    {
        return dc;
    }

private:
    HDC dc;
    HWND hwnd;
};

class BitmapSelector {
public:
    BitmapSelector(HDC hdc, HBITMAP bitmap)
        : dc(hdc)
    {
        if (!bitmap) {
            LOG_ERROR("Invalid bitmap handle");
            throw std::runtime_error("Invalid bitmap handle");
        }
        oldBitmap = (HBITMAP)SelectObject(dc, bitmap);
    }

    ~BitmapSelector()
    {
        if (oldBitmap)
            SelectObject(dc, oldBitmap);
    }

private:
    HDC dc;
    HBITMAP oldBitmap;
};

class Splash : public QSplashScreen {
public:
    explicit Splash(
        const QPixmap& pixmap, const SplashConfig& config = SplashConfig());

    void stayOnTop();
    void setOpacity(BYTE opacity);

private:
    SplashConfig config_;

    void initializeWindow(const QPixmap& pixmap);
    void updateLayeredWindow(const QPixmap& pixmap = QPixmap());
    void recover();
};

}
#endif // SPLASH_H
