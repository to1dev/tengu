#include "WalletPanel.h"

namespace Daitengu::Components {

WalletPanel::WalletPanel(QWidget* parent)
    : QFrame(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setMinimumWidth(MIN_WIDTH);
}

UserCard::UserCard(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("userCard");
    auto* mainLayout = new QVBoxLayout(this);
    auto* topLayout = new QHBoxLayout();
    auto* bottomLayout = new QHBoxLayout();
}

}
