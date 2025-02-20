#ifndef WALLETPANEL_H
#define WALLETPANEL_H

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "Consts.h"

#include "Utils/Helpers.hpp"

#include "AnimatedTabWidget.h"
#include "ClickableLabel.h"
#include "LineEditEx.h"
#include "SVGWidget.h"

using namespace Daitengu::Core;
using namespace Daitengu::Utils;

namespace Daitengu::Components {

inline constexpr char STR_NAME_LABEL[] = "nameLabel";
inline constexpr char STR_ADDRESS_LABEL[] = "addressLabel";
inline constexpr char STR_SELECT_BUTTON[] = "selectButton";
inline constexpr char STR_VALUE_LABEL[] = "valueLabel";

inline const QString STR_SELECT_BUTTON_TEXT = QObject::tr("切换");

class UserCard : public QWidget {
    Q_OBJECT

public:
    explicit UserCard(QWidget* parent = nullptr);

private:
    QLabel* nameLabel_;
    ClickableLabel* addressLabel_;
};

class ValueCard : public QWidget {
    Q_OBJECT

public:
    explicit ValueCard(QWidget* parent = nullptr);

    std::uint64_t value() const;
    void setValue(std::uint64_t newValue);

private:
    QLabel* valueLabel_;
    std::uint64_t value_ { 0 };
};

class ObjectsCard : public AnimatedTabWidget {
    Q_OBJECT

public:
    explicit ObjectsCard(QWidget* parent = nullptr);
};

class WalletPanel : public QFrame {
    Q_OBJECT

    const int MIN_WIDTH = 380;

public:
    WalletPanel(QWidget* parent = nullptr);

    UserCard* userCard() const;

    ValueCard* valueCard() const;

    ObjectsCard* objectsCard() const;

private:
    UserCard* userCard_;
    ValueCard* valueCard_;
    ObjectsCard* objectsCard_;
};

}
#endif // WALLETPANEL_H
