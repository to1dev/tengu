#include "GlobalManager.h"

namespace Daitengu::Base {

GlobalManager::GlobalManager(QApplication* app)
    : mApp(app)
{
    settingManager = std::make_unique<SettingManager>();
    themeManager = std::make_unique<ThemeManager>(app);
    windowManager = std::make_unique<WindowManager>(app);
    resourceManager = std::make_unique<ResourceManager>();
}

ResourceManager* GlobalManager::getResourceManager() const
{
    return resourceManager.get();
}

SettingManager* GlobalManager::getSettingManager() const
{
    return settingManager.get();
}

ThemeManager* GlobalManager::getThemeManager() const
{
    return themeManager.get();
}

WindowManager* GlobalManager::getWindowManager() const
{
    return windowManager.get();
}

}
