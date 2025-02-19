#include "GlobalManager.h"

namespace Daitengu::Core {

GlobalManager::GlobalManager(QApplication* app)
    : app_(app)
{
    settingManager_ = std::make_unique<SettingManager>();
    themeManager_ = std::make_unique<ThemeManager>(app);
    windowManager_ = std::make_unique<WindowManager>(app);
    resourceManager_ = std::make_unique<ResourceManager>();
}

ResourceManager* GlobalManager::resourceManager() const
{
    return resourceManager_.get();
}

SettingManager* GlobalManager::settingManager() const
{
    return settingManager_.get();
}

ThemeManager* GlobalManager::themeManager() const
{
    return themeManager_.get();
}

WindowManager* GlobalManager::windowManager() const
{
    return windowManager_.get();
}

}
