#ifndef GLOBALMANAGER_H
#define GLOBALMANAGER_H

#include "Managers/ResourceManager.h"
#include "Managers/SettingManager.h"
#include "Managers/ThemeManager.h"
#include "Managers/WindowManager.h"

namespace Daitengu::Core {

class GlobalManager {
public:
    GlobalManager(QApplication* app = nullptr);

    ThemeManager* themeManager() const;
    WindowManager* windowManager() const;
    ResourceManager* resourceManager() const;
    SettingManager* settingManager() const;

private:
    QApplication* app_;
    std::unique_ptr<ResourceManager> resourceManager_;
    std::unique_ptr<SettingManager> settingManager_;
    std::unique_ptr<ThemeManager> themeManager_;
    std::unique_ptr<WindowManager> windowManager_;
};

}
#endif // GLOBALMANAGER_H
