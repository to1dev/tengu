#include "GlobalManager.h"

namespace Daitengu::Base {

GlobalManager::GlobalManager(QApplication* app)
    : mApp(app)
{
    settingManager = std::make_unique<SettingManager>();
    themeManager = std::make_unique<ThemeManager>(app);
    layoutManager = std::make_unique<LayoutManager>(app);
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

LayoutManager* GlobalManager::getLayoutManager() const
{
    return layoutManager.get();
}

}
