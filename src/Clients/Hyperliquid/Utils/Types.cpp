#include "Types.h"

namespace hyperliquid {

hyperliquid::Cloid::Cloid(const QString& rawCloid)
    : rawCloid_(rawCloid)
{
    validate();
}

Cloid Cloid::fromInt(qint64 cloid)
{
    return Cloid(QString("0x%1").arg(
        QString::number(cloid, 16).rightJustified(32, '0')));
}

Cloid Cloid::fromStr(const QString& cloid)
{
    return Cloid(cloid);
}

void Cloid::validate()
{
    if (!rawCloid_.startsWith("0x")) {
        throw std::invalid_argument("cloid is not a hex string");
    }

    if (rawCloid_.mid(2).length() != 32) {
        throw std::invalid_argument("cloid is not 16 bytes");
    }
}
}
