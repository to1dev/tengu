#ifndef WALLETPANEL_H
#define WALLETPANEL_H

#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>

#include "Consts.h"

#include "Utils/Helpers.hpp"

using namespace Daitengu::Utils;

namespace Daitengu::Components {

class UserCard : public QWidget {
    Q_OBJECT

public:
    explicit UserCard(QWidget* parent = nullptr);
};

class WalletPanel : public QFrame {
    Q_OBJECT

    const int MIN_WIDTH = 380;

public:
    WalletPanel(QWidget* parent = nullptr);
};

}
#endif // WALLETPANEL_H
