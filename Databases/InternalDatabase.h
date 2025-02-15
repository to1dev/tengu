#ifndef INTERNALDATABASE_H
#define INTERNALDATABASE_H

#include <optional>
#include <string>

#include <QDir>
#include <QString>

#include "sqlite_orm/sqlite_orm.h"

using namespace sqlite_orm;

namespace Daitengu::Database {

inline constexpr char DB_NAME[] = "tengu";

struct Vault { };

struct SmartMoney { };

struct Address { };

struct Wallet {
    int id;
    int type;
    std::string hash;
    std::string name;
};

enum class DBErrorType {
    none = 0,
};

inline auto initStorage(const QString& dataPath)
{
    QString filePath = QDir::toNativeSeparators(
        QString("%1/%2.db").arg(dataPath).arg(DB_NAME));

    return make_storage(filePath.toStdString());
}

using Storage = decltype(initStorage(""));

static std::unique_ptr<Storage> stor;

class InternalDatabase {
public:
    InternalDatabase(const QString& dataPath);
    virtual ~InternalDatabase() = default;

    void reset();
    void vacuum();
};

}
#endif // INTERNALDATABASE_H
