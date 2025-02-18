#ifndef SPLASH_H
#define SPLASH_H

#include <memory>
#include <stdexcept>

#include <QDebug>
#include <QSplashScreen>
#include <QThread>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtWinExtras>
#endif

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
