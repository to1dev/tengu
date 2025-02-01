#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include <QDir>
#include <QStandardPaths>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

#include "Consts.h"
#include "Globals.h"

#include "Databases/InternalDatabase.h"
#include "Utils/PathUtils.hpp"

using namespace Daitengu::Database;
using namespace Daitengu::Utils;

namespace Daitengu::Base {

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

    InternalDatabase* database() const;

    QString dataPath() const;
    QString appPath() const;

    bool readSettings();
    bool writeSettings();

    Options& options();

private:
    QString mDataPath;
    QString mAppPath;
    Options mOptions;

    std::unique_ptr<InternalDatabase> mDatabase;
};

}
#endif // SETTINGMANAGER_H
