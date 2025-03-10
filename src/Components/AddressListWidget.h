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

#pragma once

#include <QApplication>
#include <QClipboard>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QStyledItemDelegate>

#include "Consts.h"

#include "Databases/Database.h"

#include "Utils/Helpers.hpp"

using namespace Daitengu::Core;
using namespace Daitengu::Databases;
using namespace Daitengu::Utils;

namespace Daitengu::Components {

inline constexpr char ADDRESS_OBJECT_NAME[] = "listWidgetAddress";
inline constexpr int ADDRESS_ICON_SIZE = 64;
inline constexpr int ADDRESS_SPACING_SIZE = 3;

class BoldFirstLineDelegate : public QStyledItemDelegate {
public:
    explicit BoldFirstLineDelegate(QObject* parent = nullptr);
    ~BoldFirstLineDelegate() override = default;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

protected:
    bool eventFilter(QObject* object, QEvent* event) override;

private:
    mutable QRect addressRect_;
    mutable bool hoverOverAddress { false };
};

class AddressListWidget : public QListWidget {
public:
    enum class ItemData {
        selected = Qt::UserRole + 200,
        id,
        type,
        walletId,
        index,
        hash,
        name,
        address,
        derivationPath,
    };

    explicit AddressListWidget(QWidget* parent = nullptr);
    ~AddressListWidget() override = default;

    void add(const Address& address, int index = 0);
    void load(const std::vector<Address>& addresses);
    void update(const Address& address);
    void purge();

    bool focusChanged();

    void setSelectedId(int newSelectedId);

private Q_SLOTS:
    void copyItemTextToClipboard(QListWidgetItem* item);

private:
    int selectedId_ { -1 };
};

}
