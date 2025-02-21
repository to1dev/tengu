#include "GlobalManager.h"

namespace Daitengu::Core {

GlobalManager::GlobalManager()
    : app_(qApp)
{
    settingManager_ = std::make_unique<SettingManager>();
    themeManager_ = std::make_unique<ThemeManager>();
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

}
