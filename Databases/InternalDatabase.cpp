#include "InternalDatabase.h"

namespace Daitengu::Database {

InternalDatabase::InternalDatabase(const QString& dataPath)
{
    stor = std::make_unique<Storage>(initStorage(dataPath));
    stor->sync_schema();
}

void InternalDatabase::reset()
{
    // Todo
    vacuum();
}

void InternalDatabase::vacuum()
{
    stor->vacuum();
}

}
