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

#include "AddressListWidget.h"

namespace Daitengu::Components {

BoldFirstLineDelegate::BoldFirstLineDelegate(QObject* parent, bool deletable)
    : QStyledItemDelegate(parent)
    , isDeletable_(deletable)
{
}

void BoldFirstLineDelegate::paint(QPainter* painter,
    const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem options = option;
    initStyleOption(&options, index);

    options.text = QString();
    options.widget->style()->drawControl(
        QStyle::CE_ItemViewItem, &options, painter, options.widget);

    const QString name
        = index.data(static_cast<int>(AddressListModel::ItemData::Name))
              .toString();
    const QString address = hideAddress(
        index.data(static_cast<int>(AddressListModel::ItemData::Address))
            .toString());

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    QFont boldFont = options.font;
    //  boldFont.setBold(true);

    QPen oldPen = painter->pen();

    QFont regularFont = options.font;
    regularFont.setUnderline(true);

    const QFontMetrics boldMetrics(boldFont);
    const QFontMetrics regularMetrics(regularFont);

    const QRect textRect = options.widget->style()->subElementRect(
        QStyle::SE_ItemViewItemText, &options);
    const int totalTextHeight
        = boldMetrics.height() + regularMetrics.height() + 8;

    const int xOffset = textRect.left() + 8;
    int yOffset = textRect.top() + (textRect.height() - totalTextHeight) / 2;
    const int availableWidth = textRect.width() - 16;

    const QString elidedName
        = boldMetrics.elidedText(name, Qt::ElideRight, availableWidth);
    const QString elidedAddress
        = regularMetrics.elidedText(address, Qt::ElideRight, availableWidth);

    // painter->setFont(boldFont);
    painter->setPen(QColor("#FFA500"));
    painter->drawText(xOffset, yOffset, availableWidth,
        boldMetrics.height() + 8, Qt::AlignLeft | Qt::AlignTop, elidedName);
    yOffset += boldMetrics.height() + 8;

#ifdef onpaint
    {
        const int addressWidth = regularMetrics.horizontalAdvance(address);
        const int actualWidth = qMin(addressWidth, availableWidth);
        addressRect_
            = QRect(xOffset, yOffset, actualWidth, regularMetrics.height());
    }
#endif

    painter->setPen(oldPen);
    painter->setFont(regularFont);
    painter->drawText(xOffset, yOffset, availableWidth, regularMetrics.height(),
        Qt::AlignLeft | Qt::AlignTop, elidedAddress);

    const int buttonSize = 16;
    const int buttonMargin = 8;

    const QRect buttonRect(textRect.right() - buttonSize - buttonMargin,
        textRect.top() + (textRect.height() - buttonSize) / 2, buttonSize,
        buttonSize);

    if (isDeletable_ && option.state & QStyle::State_MouseOver) {
        deleteButtonSvg_->render(painter, buttonRect);
    }

    painter->restore();
}

bool BoldFirstLineDelegate::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::MouseMove) {
        if (auto* mouseEvent = dynamic_cast<QMouseEvent*>(event)) {
            auto* listView = qobject_cast<QListView*>(parent());
            if (listView) {
                const QModelIndex index = listView->indexAt(mouseEvent->pos());
                if (index.isValid()) {
                    const QRect rect = listView->visualRect(index);

                    const QFontMetrics metrics(listView->font());
                    const QString address = hideAddress(index
                            .data(static_cast<int>(
                                AddressListModel::ItemData::Address))
                            .toString());
                    const int availableWidth = rect.width() - 16;
                    const int addressWidth = metrics.horizontalAdvance(address);
                    const int actualWidth = qMin(addressWidth, availableWidth);

                    const QRect addressRect(rect.left() + 8,
                        rect.top() + metrics.height() + 16, actualWidth,
                        metrics.height());

                    const int buttonSize = 16;
                    const int buttonMargin = 8;
                    const QRect deleteButtonRect(
                        rect.right() - buttonSize - buttonMargin,
                        rect.top() + (rect.height() - buttonSize) / 2,
                        buttonSize, buttonSize);

                    hoverOverAddress = addressRect.contains(mouseEvent->pos());
                    hoverOverDeleteButton = isDeletable_
                        && deleteButtonRect.contains(mouseEvent->pos());

                    listView->viewport()->setCursor(
                        (hoverOverAddress || hoverOverDeleteButton)
                            ? Qt::PointingHandCursor
                            : Qt::ArrowCursor);
                    return true;
                }
            }
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        if (auto* mouseEvent = dynamic_cast<QMouseEvent*>(event)) {
            if (mouseEvent->button() == Qt::LeftButton) {
                auto* listView = qobject_cast<QListView*>(parent());
                if (listView) {
                    const QModelIndex index
                        = listView->indexAt(mouseEvent->pos());
                    if (index.isValid()) {
                        if (hoverOverAddress) {
                            clip::set_text(index
                                    .data(static_cast<int>(
                                        AddressListModel::ItemData::Address))
                                    .toString()
                                    .toStdString());
                            return true;
                        } else if (hoverOverDeleteButton) {
                            Q_EMIT deleteRequested(index);
                            return true;
                        }
                    }
                }
            }
        }
    } else if (event->type() == QEvent::MouseButtonDblClick) {
        if (auto* mouseEvent = dynamic_cast<QMouseEvent*>(event)) {
            if (mouseEvent->button() == Qt::LeftButton) {
                auto* listView = qobject_cast<QListView*>(parent());
                if (listView) {
                    const QModelIndex index
                        = listView->indexAt(mouseEvent->pos());
                    if (index.isValid()) {
                        if (hoverOverAddress || hoverOverDeleteButton) {
                            return true;
                        }
                    }
                }
            }
        }
    }

    return QObject::eventFilter(object, event);
}

AddressListModel::AddressListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int AddressListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(addresses_.size());
}

QVariant AddressListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0
        || index.row() >= static_cast<int>(addresses_.size()))
        return QVariant();

    const Address& addr = addresses_[index.row()];
    switch (role) {
    case Qt::DisplayRole:
        return "Dummy Text First\nSecond";
    case static_cast<int>(ItemData::Id):
        return addr.id;
    case static_cast<int>(ItemData::Type):
        return addr.type;
    case static_cast<int>(ItemData::WalletId):
        return addr.walletId;
    case static_cast<int>(ItemData::Index):
        return addr.index;
    case static_cast<int>(ItemData::Hash):
        return QString::fromStdString(addr.hash);
    case static_cast<int>(ItemData::Name):
        return QString::fromStdString(addr.name);
    case static_cast<int>(ItemData::Address):
        return QString::fromStdString(addr.address);
    case static_cast<int>(ItemData::DerivationPath):
        return QString::fromStdString(addr.derivationPath);
    case static_cast<int>(ItemData::PrivateKey):
        return QString::fromStdString(addr.privateKey);
    default:
        return QVariant();
    }
}

void AddressListModel::add(const Address& address)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    addresses_.push_back(address);
    endInsertRows();
}

void AddressListModel::load(const std::vector<Address>& addresses)
{
    beginResetModel();
    addresses_ = addresses;
    endResetModel();
}

void AddressListModel::update(const Address& address)
{
    for (size_t i = 0; i < addresses_.size(); ++i) {
        if (addresses_[i].id == address.id) {
            addresses_[i] = address;
            QModelIndex idx = index(static_cast<int>(i));
            Q_EMIT dataChanged(idx, idx);
            break;
        }
    }
}

void AddressListModel::purge()
{
    beginResetModel();
    addresses_.clear();
    endResetModel();
}

void AddressListModel::remove(const QList<int>& rows)
{
    QList<int> sortedRows = rows;
    std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());
    for (int row : sortedRows) {
        if (row < 0 || row >= static_cast<int>(addresses_.size()))
            continue;
        beginRemoveRows(QModelIndex(), row, row);
        addresses_.erase(addresses_.begin() + row);
        endRemoveRows();
    }
}

AddressListView::AddressListView(QWidget* parent, bool deletable)
    : QListView(parent)
{
    setObjectName(ADDRESS_OBJECT_NAME);
    setIconSize(QSize(ADDRESS_ICON_SIZE, ADDRESS_ICON_SIZE));
    setSpacing(ADDRESS_SPACING_SIZE);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setMouseTracking(true);

    model_ = new AddressListModel(this);
    setModel(model_);

    auto* delegate = new BoldFirstLineDelegate(this, deletable);
    setItemDelegate(delegate);

    viewport()->installEventFilter(delegate);

    connect(delegate, &BoldFirstLineDelegate::deleteRequested, this,
        [this](const QModelIndex& index) { Q_EMIT deleteRequested(index); });
}

void AddressListView::add(const Address& address)
{
    model_->add(address);
}

void AddressListView::load(const std::vector<Address>& addresses)
{
    model_->load(addresses);
}

void AddressListView::update(const Address& address)
{
    model_->update(address);
}

void AddressListView::purge()
{
    model_->purge();
}

void AddressListView::remove(const QList<int>& rows)
{
    model_->remove(rows);
}

}
