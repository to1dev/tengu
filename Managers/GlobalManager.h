#ifndef GLOBALMANAGER_H
#define GLOBALMANAGER_H

#include "Managers/ResourceManager.h"
#include "Managers/SettingManager.h"
#include "Managers/ThemeManager.h"
#include "Managers/WindowManager.h"

namespace Daitengu::Base {

class GlobalManager {
public:
    GlobalManager(QApplication* app = nullptr);

    ThemeManager* getThemeManager() const;
    WindowManager* getWindowManager() const;
    ResourceManager* getResourceManager() const;
    SettingManager* getSettingManager() const;

private:
    QApplication* mApp;
    std::unique_ptr<ResourceManager> resourceManager;
    std::unique_ptr<SettingManager> settingManager;
    std::unique_ptr<ThemeManager> themeManager;
    std::unique_ptr<WindowManager> windowManager;
};

}
#endif // GLOBALMANAGER_H
