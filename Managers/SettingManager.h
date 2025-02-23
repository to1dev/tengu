#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include <QDir>
#include <QStandardPaths>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

#include "Consts.h"
#include "Globals.h"

#include "Databases/Database.h"
#include "Utils/PathUtils.hpp"

using namespace Daitengu::Core;
using namespace Daitengu::Databases;
using namespace Daitengu::Utils;

namespace Daitengu::Core {

inline constexpr char STR_VAULT_OPTIONS[] = "Vault";
inline constexpr char STR_VAULT_ID[] = "vaultId";
inline constexpr char STR_VAULT_TYPE[] = "vaultType";
inline constexpr char STR_ADDRESS_ID[] = "id";
inline constexpr char STR_ADDRESS_TYPE[] = "type";

struct SystemOptions {
    QString machineId;
    QString appPath;
    double deviceRatio;
    QString dpiSuffix;
};
struct Options {
    SystemOptions sysOpt;
};

class SettingManager {
public:
    SettingManager();
    ~SettingManager();

    Database* database() const;

    QString dataPath() const;
    QString appPath() const;

    bool readSettings();
    bool writeSettings();

    Options& options();

private:
    QString dataPath_;
    QString appPath_;
    Options options_;

    std::unique_ptr<Database> database_;
};

}
#endif // SETTINGMANAGER_H
