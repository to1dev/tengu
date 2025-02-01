#include "Tengu.h"

#include <iostream>

#include <QApplication>

#include "Consts.h"

#include "Components/Splash.h"

#include "Managers/GlobalManager.h"
#include "Managers/SettingManager.h"
#include "Managers/ThemeManager.h"

#include "Utils/Encryption.h"
#include "Utils/RunGuard.h"

using Guard = Daitengu::Utils::RunGuard;

using namespace Daitengu::Base;
using namespace Daitengu::Components;
using namespace Daitengu::Utils;

#ifdef USE_TEST
#include "catch_amalgamated.hpp"
#endif

int main(int argc, char* argv[])
{
    int currentExitCode = 0;

    QByteArray platform = "windows:fontengine=freetype";
    qputenv("QT_QPA_PLATFORM", platform);

    if (!Encryption::init())
        return -1;

    Guard guard;
    if (!guard.tryToRun())
        return currentExitCode;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#ifdef USE_HDPI
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QCoreApplication::setAttribute(Qt::AA_Use96Dpi);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#ifdef USE_TEST
    std::cout << std::unitbuf;
    Catch::Session().run(argc, argv);
    std::cout << std::nounitbuf;
#endif

    do {
        QApplication a(argc, argv);

        Splash splash(QPixmap(QString(SPLASH_PATH).arg(randomIndex(1, 2))));
        splash.stayOnTop();
        splash.show();

        a.processEvents();

        std::shared_ptr<GlobalManager> globalManager
            = std::make_shared<GlobalManager>(&a);

        Tengu w(globalManager);

        SplashThread::sleep(SLEEP_TIME);
        splash.hide();

        w.show();

        splash.finish(&w);
        splash.close();

        currentExitCode = a.exec();
    } while (currentExitCode == EXIT_CODE_REBOOT);

    return currentExitCode;
}
