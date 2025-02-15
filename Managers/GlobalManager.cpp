#include "GlobalManager.h"

namespace Daitengu::Base {

GlobalManager::GlobalManager(QApplication* app)
    : app_(app)
{
    settingManager_ = std::make_unique<SettingManager>();
    themeManager_ = std::make_unique<ThemeManager>(app);
    layoutManager_ = std::make_unique<LayoutManager>(app);
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

LayoutManager* GlobalManager::layoutManager() const
{
    return layoutManager_.get();
}

}
