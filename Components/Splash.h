#ifndef SPLASH_H
#define SPLASH_H

#include <QSplashScreen>
#include <QThread>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtWinExtras>
#endif

#include <windows.h>

namespace Daitengu::Components {

inline constexpr int SLEEP_TIME = 1;

class SplashThread : public QThread {
public:
    static void sleep(unsigned long secs)
    {
        QThread::sleep(secs);
    }
};

class Splash : public QSplashScreen {
public:
    Splash(const QPixmap& pixmap = QPixmap());

    void stayOnTop();
};

}
#endif // SPLASH_H
