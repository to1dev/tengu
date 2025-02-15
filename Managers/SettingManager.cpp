#include "SettingManager.h"

namespace Daitengu::Base {

SettingManager::SettingManager()
{
    dataPath_ = QDir::toNativeSeparators(QString("%1/%2/%3")
            .arg(QStandardPaths::writableLocation(
                QStandardPaths::GenericDataLocation))
            .arg(COMPANY)
            .arg(NAME));

    if (!QDir(dataPath_).exists())
        QDir().mkpath(dataPath_);

    // QString path = QDir::toNativeSeparators(QString(argv[0]));
    // mOptions.sysOpt.appPath = mAppPath = QFileInfo(path).absolutePath();

    options_.sysOpt.appPath = appPath_
        = QString::fromStdString(PathUtils::getExecutableDir().string());

    database_ = std::make_unique<Database>(dataPath_);

    readSettings();
}

SettingManager::~SettingManager()
{
    writeSettings();
}

Database* SettingManager::database() const
{
    return database_.get();
}

QString SettingManager::dataPath() const
{
    return dataPath_;
}

QString SettingManager::appPath() const
{
    return appPath_;
}

bool SettingManager::readSettings()
{
    bool ret = false;

    return ret;
}

bool SettingManager::writeSettings()
{
    bool ret = false;

    return ret;
}

Options& SettingManager::options()
{
    return options_;
}

}
