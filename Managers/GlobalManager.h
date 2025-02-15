#ifndef GLOBALMANAGER_H
#define GLOBALMANAGER_H

#include "Managers/LayoutManager.h"
#include "Managers/ResourceManager.h"
#include "Managers/SettingManager.h"
#include "Managers/ThemeManager.h"

namespace Daitengu::Core {

class GlobalManager {
public:
    GlobalManager(QApplication* app = nullptr);

    ThemeManager* themeManager() const;
    LayoutManager* layoutManager() const;
    ResourceManager* resourceManager() const;
    SettingManager* settingManager() const;

private:
    QApplication* app_;
    std::unique_ptr<ResourceManager> resourceManager_;
    std::unique_ptr<SettingManager> settingManager_;
    std::unique_ptr<ThemeManager> themeManager_;
    std::unique_ptr<LayoutManager> layoutManager_;
};

}
#endif // GLOBALMANAGER_H
