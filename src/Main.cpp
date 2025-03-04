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

#include "Tengu.h"

#include <iostream>

#include <QApplication>

#ifdef WIN32
#include <windows.h>
#endif

#include "Consts.h"

#include "Components/Splash.h"

#include "Managers/GlobalManager.h"
#include "Managers/SettingManager.h"
#include "Managers/ThemeManager.h"
#include "Managers/WindowManager.h"

#include "Forms/WalletDock.h"

#include "Utils/Helpers.hpp"
#include "Utils/RunGuard.h"

using Guard = Daitengu::Utils::RunGuard;

using namespace Daitengu::Core;
using namespace Daitengu::Components;
using namespace Daitengu::Utils;

#ifdef USE_TEST
#include "catch_amalgamated.hpp"
#endif

int main(int argc, char* argv[])
{
#ifdef WIN32
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        FILE* fp = nullptr;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
    }
#endif

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
            = std::make_shared<GlobalManager>();

        Tengu w(globalManager);

        std::shared_ptr<WalletDock> walletDock
            = std::make_shared<WalletDock>(&w, globalManager);
        w.setWalletDock(walletDock);

        SplashThread::sleep(SLEEP_TIME);
        splash.hide();

        w.show();
        walletDock->show();

        splash.finish(&w);
        splash.close();

        currentExitCode = a.exec();
    } while (currentExitCode == EXIT_CODE_REBOOT);

    return currentExitCode;
}
