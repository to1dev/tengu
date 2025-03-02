#include "WalletDock.h"

WalletDock::WalletDock(QWidget* parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    resize(400, 800);
}
