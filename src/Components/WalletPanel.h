// SPDX-License-Identifier: AGPL-3.0-or-later
/*
 * Copyright (C) 2025 to1dev <https://arc20.me/to1dev>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef WALLETPANEL_H
#define WALLETPANEL_H

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "clip.h"

#include "Consts.h"

#include "Utils/Helpers.hpp"

#include "Databases/Database.h"

#include "AnimatedTabWidget.h"
#include "CircularButton.h"
#include "ClickableLabel.h"
#include "LineEditEx.h"
#include "SVGWidget.h"

using namespace Daitengu::Core;
using namespace Daitengu::Databases;
using namespace Daitengu::Utils;

namespace Daitengu::Components {

class UserCard : public QWidget {
    Q_OBJECT

public:
    explicit UserCard(QWidget* parent = nullptr);
    void reset(int walletId, int id);
    void update(const std::optional<Wallet>& wallet,
        const std::optional<Address>& address);

    void setRecord(Record&& record);
    const Record& record_ref() const;
    Record record() const;

Q_SIGNALS:
    void onSelect();
    void onPopup(const QPoint& pt);

private:
    QLabel* nameLabel_;
    ClickableLabel* addressLabel_;

    Record record_;
};

class ValueCard : public QWidget {
    Q_OBJECT

public:
    explicit ValueCard(QWidget* parent = nullptr);

    QString value() const;
    void setValue(const QString& newValue);

private:
    QLabel* valueLabel_;
    QString value_;
};

class ObjectsCard : public AnimatedTabWidget {
    Q_OBJECT

public:
    explicit ObjectsCard(QWidget* parent = nullptr);
};

class WalletPanel : public QFrame {
    Q_OBJECT

    const int MIN_WIDTH = 400;

public:
    explicit WalletPanel(QWidget* parent = nullptr);

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
