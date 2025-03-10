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

BoldFirstLineDelegate::BoldFirstLineDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
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
        = index.data(static_cast<int>(AddressListWidget::ItemData::name))
              .toString();
    const int i
        = index.data(static_cast<int>(AddressListWidget::ItemData::index))
              .toInt()
        + 1;
    const QString address = hideAddress(
        index.data(static_cast<int>(AddressListWidget::ItemData::address))
            .toString());

    const QString displayName
        = name.isEmpty() ? QString(STR_ADDRESS_NAME).arg(i) : name;

    painter->save();

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
        = boldMetrics.elidedText(displayName, Qt::ElideRight, availableWidth);
    const QString elidedAddress
        = regularMetrics.elidedText(address, Qt::ElideRight, availableWidth);

    // painter->setFont(boldFont);
    painter->setPen(QColor("#FFA500"));
    painter->drawText(xOffset, yOffset, availableWidth,
        boldMetrics.height() + 8, Qt::AlignLeft | Qt::AlignTop, elidedName);
    yOffset += boldMetrics.height() + 8;

    {
        // Calc address rect
        const int addressWidth = regularMetrics.horizontalAdvance(address);
        const int actualWidth = qMin(addressWidth, availableWidth);
        const QRect addressRect(
            xOffset, yOffset, actualWidth, regularMetrics.height());
        addressRect_ = addressRect;
    }

    painter->setPen(oldPen);
    painter->setFont(regularFont);
    painter->drawText(xOffset, yOffset, availableWidth, regularMetrics.height(),
        Qt::AlignLeft | Qt::AlignTop, elidedAddress);

    painter->restore();
}

bool BoldFirstLineDelegate::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::MouseMove) {
        if (auto* mouseEvent = dynamic_cast<QMouseEvent*>(event)) {
            auto* listView = qobject_cast<QListView*>(parent());
            if (listView) {
                const QModelIndex index = listView->indexAt(mouseEvent->pos());
                const QRect rect = listView->visualRect(index);
                const QFontMetrics metrics(listView->font());
#ifdef original
                const QRect addressRect(rect.left() + 8,
                    rect.top() + metrics.height() + 16, rect.width() - 16,
                    metrics.height());
#endif

                hoverOverAddress = addressRect_.contains(mouseEvent->pos());
                listView->viewport()->setCursor(hoverOverAddress
                        ? Qt::PointingHandCursor
                        : Qt::ArrowCursor);
                return true;
            }
        }
    } else if (event->type() == QEvent::MouseButtonPress && hoverOverAddress) {
        if (auto* mouseEvent = dynamic_cast<QMouseEvent*>(event)) {
            if (mouseEvent->button() == Qt::LeftButton) {
                auto* listView = qobject_cast<QListView*>(parent());
                if (listView) {
                    const QModelIndex index
                        = listView->indexAt(mouseEvent->pos());
                    const QRect rect = listView->visualRect(index);
                    const QFontMetrics metrics(listView->font());
                    const QRect addressRect(rect.left() + 8,
                        rect.top() + metrics.height() + 16, rect.width() - 16,
                        metrics.height());

                    if (addressRect.contains(mouseEvent->pos())) {
                        QApplication::clipboard()->setText(index
                                .data(static_cast<int>(
                                    AddressListWidget::ItemData::address))
                                .toString());
                    }
                }
            }
        }
    }

    return QObject::eventFilter(object, event);
}

AddressListWidget::AddressListWidget(QWidget* parent)
    : QListWidget(parent)
{
    setObjectName(ADDRESS_OBJECT_NAME);
    setIconSize(QSize(ADDRESS_ICON_SIZE, ADDRESS_ICON_SIZE));
    setSpacing(ADDRESS_SPACING_SIZE);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    setContextMenuPolicy(Qt::CustomContextMenu);

    auto* delegate = new BoldFirstLineDelegate(this);
    setItemDelegate(delegate);
    viewport()->installEventFilter(delegate);

    /*connect(this, &QListWidget::itemDoubleClicked, this,
        &AddressListWidget::copyItemTextToClipboard);*/
}

void AddressListWidget::add(const Address& address, int index)
{
    auto* item = new QListWidgetItem();

    item->setText("Dummy Text First\nSecond");

    item->setData(static_cast<int>(ItemData::selected), false);
    item->setData(static_cast<int>(ItemData::id), address.id);
    item->setData(static_cast<int>(ItemData::type), address.type);
    item->setData(static_cast<int>(ItemData::walletId), address.walletId);
    item->setData(static_cast<int>(ItemData::index), index);
    item->setData(
        static_cast<int>(ItemData::hash), QString::fromStdString(address.hash));
    item->setData(
        static_cast<int>(ItemData::name), QString::fromStdString(address.name));
    item->setData(static_cast<int>(ItemData::address),
        QString::fromStdString(address.address));
    item->setData(static_cast<int>(ItemData::derivationPath),
        QString::fromStdString(address.derivationPath));

    addItem(item);

    if (selectedId_ == address.id) {
        item->setSelected(true);
        setCurrentRow(row(item));
        scrollToItem(item);
    }
}

void AddressListWidget::load(const std::vector<Address>& addresses)
{
    setUpdatesEnabled(false);

    clear();
    int index = 0;
    for (const auto& address : addresses) {
        add(address, index++);
    }

    setUpdatesEnabled(true);
}

void AddressListWidget::update(const Address& address)
{
    QListWidgetItem* item = currentItem();
    if (!item)
        return;

    item->setData(
        static_cast<int>(ItemData::name), QString::fromStdString(address.name));
    item->setData(static_cast<int>(ItemData::address),
        QString::fromStdString(address.address));
}

void AddressListWidget::purge()
{
    while (!selectedItems().isEmpty()) {
        delete takeItem(row(selectedItems().first()));
    }
}

bool AddressListWidget::focusChanged()
{
    return currentItem() ? !currentItem()->isSelected() : false;
}

void AddressListWidget::setSelectedId(int newSelectedId)
{
    selectedId_ = newSelectedId;
}

void AddressListWidget::copyItemTextToClipboard(QListWidgetItem* item)
{
    if (item) {
        const QString address
            = item->data(static_cast<int>(AddressListWidget::ItemData::address))
                  .toString();
        QApplication::clipboard()->setText(address);
    }
}

}
