#ifndef GLOBALMANAGER_H
#define GLOBALMANAGER_H

#include "Managers/ResourceManager.h"
#include "Managers/SettingManager.h"
#include "Managers/ThemeManager.h"

namespace Daitengu::Core {

class GlobalManager {
public:
    GlobalManager();

    ThemeManager* themeManager() const;
    ResourceManager* resourceManager() const;
    SettingManager* settingManager() const;

private:
    QApplication* app_;
    std::unique_ptr<ResourceManager> resourceManager_;
    std::unique_ptr<SettingManager> settingManager_;
    std::unique_ptr<ThemeManager> themeManager_;
};

}
#endif // GLOBALMANAGER_H
