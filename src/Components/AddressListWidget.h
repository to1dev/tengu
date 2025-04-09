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

#include <vector>

#include <QAbstractListModel>
#include <QListView>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QStyledItemDelegate>
#include <QSvgRenderer>

#include "clip.h"

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
    Q_OBJECT

public:
    explicit BoldFirstLineDelegate(
        QObject* parent = nullptr, bool deletable = false);
    ~BoldFirstLineDelegate() override = default;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

protected:
    bool eventFilter(QObject* object, QEvent* event) override;

Q_SIGNALS:
    void deleteRequested(const QModelIndex& index) const;

private:
    mutable bool hoverOverAddress { false };
    mutable bool hoverOverDeleteButton { false };
    std::unique_ptr<QSvgRenderer> deleteButtonSvg_ {
        std::make_unique<QSvgRenderer>(QString(":/Media/Xmark"))
    };

    bool isDeletable_ { false };
};

class AddressListModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum class ItemData {
        Id = Qt::UserRole + 200,
        Type,
        WalletId,
        Index,
        Hash,
        Name,
        Address,
        DerivationPath,
        PrivateKey,
    };

    explicit AddressListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(
        const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void add(const Address& address);
    void load(const std::vector<Address>& addresses);
    void update(const Address& address);
    void purge();
    void remove(const QList<int>& rows);

private:
    std::vector<Address> addresses_;
};

class AddressListView : public QListView {
    Q_OBJECT

public:
    explicit AddressListView(QWidget* parent = nullptr, bool deletable = false);

    void add(const Address& address);
    void load(const std::vector<Address>& addresses);
    void update(const Address& address);
    void purge();
    void remove(const QList<int>& rows);

    AddressListModel* model() const
    {
        return model_;
    }

Q_SIGNALS:
    void deleteRequested(const QModelIndex& index);
    void addressDoubleClicked(int id);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    AddressListModel* model_;
};

}
