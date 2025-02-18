#include "Splash.h"

namespace Daitengu::Components {

SplashThread::SplashThread(QObject* parent)
    : QThread(parent)
{
}

void SplashThread::sleep(unsigned long secs)
{
    QThread::sleep(secs);
}

void SplashThread::msleep(unsigned long msecs)
{
    QThread::msleep(msecs);
}

void SplashThread::usleep(unsigned long usecs)
{
    QThread::usleep(usecs);
}

bool SplashThread::interruptibleSleep(unsigned long secs)
{
    const unsigned long checkInterval = 100;
    unsigned long remainingTime = secs * 1000;

    while (remainingTime > 0 && !isInterruptionRequested()) {
        unsigned long sleepTime = std::min(remainingTime, checkInterval);
        msleep(sleepTime);
        remainingTime -= sleepTime;
    }

    return !isInterruptionRequested();
}

void SplashThread::run()
{
    Q_EMIT started();
    exec();
    Q_EMIT finished();
}

void SplashThread::requestStop()
{
    requestInterruption();
    quit();
    wait();
}

SplashThreadEx::SplashThreadEx(QObject* parent)
    : SplashThread(parent)
    , isPaused_(false)
{
}

void SplashThreadEx::pause()
{
    mutex_.lock();
    isPaused_ = true;
    mutex_.unlock();
}

void SplashThreadEx::resume()
{
    mutex_.lock();
    isPaused_ = false;
    condition_.wakeAll();
    mutex_.unlock();
}

bool SplashThreadEx::interruptibleSleepWithTimeout(
    unsigned long secs, unsigned long timeoutSecs)
{
    QDeadlineTimer deadline(timeoutSecs * 1000);

    const unsigned long checkInterval = 100;
    unsigned long remainingTime = secs * 1000;

    while (remainingTime > 0 && !isInterruptionRequested()) {
        if (deadline.hasExpired()) {
            Q_EMIT error("Operation timed out");
            return false;
        }

        checkPause();

        unsigned long sleepTime = std::min(remainingTime, checkInterval);
        msleep(sleepTime);
        remainingTime -= sleepTime;
    }

    return !isInterruptionRequested();
}

void SplashThreadEx::checkPause()
{
    mutex_.lock();
    while (isPaused_ && !isInterruptionRequested()) {
        condition_.wait(&mutex_);
    }
    mutex_.unlock();
}

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
HBITMAP QImageToHBITMAP(const QImage& image, bool premultiplied = false)
{
    try {
        QImage convertedImage = premultiplied
            ? image.convertToFormat(QImage::Format_ARGB32_Premultiplied)
            : image.convertToFormat(QImage::Format_RGBA8888);

        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = convertedImage.width();
        bmi.bmiHeader.biHeight = -convertedImage.height();
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        DCWrapper hdc;
        HBITMAP hBitmap = CreateDIBitmap(hdc, &bmi.bmiHeader, CBM_INIT,
            convertedImage.constBits(), &bmi, DIB_RGB_COLORS);

        if (!hBitmap) {
            LOG_ERROR("Failed to create bitmap");
            throw std::runtime_error("Failed to create bitmap");
        }

        return hBitmap;
    } catch (const std::exception& e) {
        LOG_ERROR(QString("QImageToHBITMAP failed: %1").arg(e.what()));
        throw;
    }
}

HBITMAP QImageToHBITMAPWithPremultipliedAlpha(const QImage& image)
{
    return QImageToHBITMAP(image, true);
}
#endif

Splash::Splash(const QPixmap& pixmap, const SplashConfig& config)
    : QSplashScreen(pixmap)
    , config_(config)
{
    LOG_INFO("Initializing Splash Screen");
    try {
        initializeWindow(pixmap);
    } catch (const std::exception& e) {
        LOG_ERROR(
            QString("Failed to initialize splash screen: %1").arg(e.what()));
        recover();
    }
}

void Splash::stayOnTop()
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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        auto hBitmap
            = QtWin::toHBITMAP(pixmap.isNull() ? this->pixmap() : pixmap,
                QtWin::HBitmapPremultipliedAlpha);
#else
        auto hBitmap = QImageToHBITMAPWithPremultipliedAlpha(
            (pixmap.isNull() ? this->pixmap() : pixmap).toImage());
#endif

        BitmapSelector selector(dcMem.get(), hBitmap);

        if (!UpdateLayeredWindow(handle, dcScreen, &ptDst, &size, dcMem.get(),
                &ptSrc, 0, &bf, config_.layeredFlags)) {
            LOG_ERROR("UpdateLayeredWindow failed");
        }

        DeleteObject(hBitmap);
    } catch (const std::exception& e) {
        LOG_ERROR(QString("updateLayeredWindow failed: %1").arg(e.what()));
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

}
