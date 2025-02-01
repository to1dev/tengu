#include "SettingManager.h"

namespace Daitengu::Base {

SettingManager::SettingManager()
{
    mDataPath = QDir::toNativeSeparators(QString("%1/%2/%3")
            .arg(QStandardPaths::writableLocation(
                QStandardPaths::GenericDataLocation))
            .arg(COMPANY)
            .arg(NAME));

    if (!QDir(mDataPath).exists())
        QDir().mkpath(mDataPath);

    // QString path = QDir::toNativeSeparators(QString(argv[0]));
    // mOptions.sysOpt.appPath = mAppPath = QFileInfo(path).absolutePath();

    mOptions.sysOpt.appPath = mAppPath
        = QString::fromStdString(PathUtils::getExecutableDir().string());

    mDatabase = std::make_unique<InternalDatabase>(mDataPath);

    readSettings();
}

SettingManager::~SettingManager()
{
    writeSettings();
}

InternalDatabase* SettingManager::database() const
{
    return mDatabase.get();
}

QString SettingManager::dataPath() const
{
    return mDataPath;
}

QString SettingManager::appPath() const
{
    return mAppPath;
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
    return mOptions;
}

}
