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

#include "WalletListWidget.h"

namespace Daitengu::Components {

BadgeItemDelegate::BadgeItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void BadgeItemDelegate::paint(QPainter* painter,
    const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    int group
        = index.data(static_cast<int>(WalletListModel::ItemData::GroupType))
              .toInt();
    int type
        = index.data(static_cast<int>(WalletListModel::ItemData::Type)).toInt();

    if (group > 0 && type > 0) {
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::TextAntialiasing);
        painter->setRenderHint(QPainter::SmoothPixmapTransform);

        QIcon badgeIcon(QString(BADGE_ICON).arg(type));
        QSize badgeSize(32, 32);
        QPoint badgePosition(option.rect.topRight().x() - badgeSize.width() - 3,
            option.rect.topRight().y() + 3);
        /*QPoint badgePosition(
            option.rect.topLeft().x(), option.rect.topRight().y() - 6);*/
        QRect badgeRect(badgePosition, badgeSize);

        painter->save();
        badgeIcon.paint(painter, badgeRect);
        painter->restore();
    }
}

WalletTypeBadgeDelegate::WalletTypeBadgeDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void WalletTypeBadgeDelegate::paint(QPainter* painter,
    const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    int group
        = index.data(static_cast<int>(WalletListModel::ItemData::GroupType))
              .toInt();
    int type
        = index.data(static_cast<int>(WalletListModel::ItemData::Type)).toInt();

    if (group > 0 && type > 0) {
        // bool isSelected = option.state & QStyle::State_Selected;
        // int alpha = isSelected ? 200 : 220;

        QString badgeText
            = QString::fromStdString(std::string(WalletTypeText.at(type)));

        painter->setRenderHint(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::TextAntialiasing);
        painter->setRenderHint(QPainter::SmoothPixmapTransform);

        int badgeSize = qMin(option.rect.width() / 3, 24);
        int badgeX = option.rect.right() - badgeSize - 6;
        int badgeY = option.rect.top() + 6;
        QRect badgeRect(badgeX, badgeY, badgeSize, badgeSize);

        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(QColor(255, 0, 0, 200)));
        painter->drawEllipse(badgeRect);

        painter->setPen(QPen(QColor(255, 255, 255, 200)));
        QFont font = painter->font();
        painter->setFont(font);
        painter->drawText(badgeRect, Qt::AlignCenter, badgeText);
    }
}

WalletListModel::WalletListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int WalletListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(wallets_.size());
}

QVariant WalletListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0
        || index.row() >= static_cast<int>(wallets_.size()))
        return QVariant();

    const Wallet& wallet = wallets_[index.row()];
    switch (role) {
    case Qt::DisplayRole:
        return QString::fromStdString(wallet.name);
    case Qt::DecorationRole: {
        int group = wallet.groupType;
        int chain = wallet.chainType;
        QIcon icon(QString::fromUtf8(
            WalletListIcons[group].items[chain].icon.data(),
            static_cast<int>(WalletListIcons[group].items[chain].icon.size())));
        return icon;
    }
    case Qt::ToolTipRole:
        return QString::fromStdString(wallet.name);
    case Id:
        return wallet.id;
    case Type:
        return wallet.type;
    case GroupType:
        return wallet.groupType;
    case ChainType:
        return wallet.chainType;
    case NetworkType:
        return wallet.networkType;
    case Index:
        return index.row();
    case Hash:
        return QString::fromStdString(wallet.hash);
    case Name:
        return QString::fromStdString(wallet.name);
    case Mnemonic:
        return QString::fromStdString(wallet.mnemonic);
    default:
        return QVariant();
    }
}

void WalletListModel::add(const Wallet& wallet)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    wallets_.push_back(wallet);
    endInsertRows();
}

void WalletListModel::load(const std::vector<Wallet>& wallets)
{
    beginResetModel();
    wallets_ = wallets;
    endResetModel();
}

void WalletListModel::update(const Wallet& wallet)
{
    for (size_t i = 0; i < wallets_.size(); ++i) {
        if (wallets_[i].id == wallet.id) {
            wallets_[i] = wallet;
            QModelIndex idx = index(static_cast<int>(i));
            Q_EMIT dataChanged(idx, idx);
            break;
        }
    }
}

void WalletListModel::remove(const QList<int>& rows)
{
    QList<int> sortedRows = rows;
    std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());
    for (int row : sortedRows) {
        if (row < 0 || row >= static_cast<int>(wallets_.size()))
            continue;
        beginRemoveRows(QModelIndex(), row, row);
        wallets_.erase(wallets_.begin() + row);
        endRemoveRows();
    }
}

WalletListView::WalletListView(QWidget* parent)
    : QListView(parent)
{
    setObjectName(WALLET_OBJECT_NAME);
    setViewMode(QListView::IconMode);
    setIconSize(QSize(WALLET_ICON_SIZE, WALLET_ICON_SIZE));
    setResizeMode(QListView::Adjust);
    setSpacing(WALLET_SPACING_SIZE);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    setDragEnabled(false);
    setContextMenuPolicy(Qt::CustomContextMenu);

    setItemDelegate(new WalletTypeBadgeDelegate(this));

    model_ = new WalletListModel(this);
    setModel(model_);
}

void WalletListView::load(const std::vector<Wallet>& wallets)
{
    model_->load(wallets);
}

void WalletListView::add(const Wallet& wallet)
{
    model_->add(wallet);
}

void WalletListView::update(const Wallet& wallet)
{
    model_->update(wallet);
}

void WalletListView::remove(const QList<int>& rows)
{
    model_->remove(rows);
}

}
