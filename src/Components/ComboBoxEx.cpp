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

#include "ComboBoxEx.h"

PopupListWidget::PopupListWidget(QWidget* parent)
    : QListWidget(parent)
{
    setObjectName("popupList");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
}

void PopupListWidget::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        hide();
        event->accept();
        return;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (currentItem()) {
            Q_EMIT enterKeyPressed(currentItem());
            event->accept();
            return;
        }
        break;

    default:
        break;
    }

    QListWidget::keyPressEvent(event);
}

ComboBoxEx::ComboBoxEx(QWidget* parent)
    : QLineEdit(parent)
    , listWidget_(new PopupListWidget(nullptr))
    , currentIndex_(-1)
{
    setReadOnly(true);

    listWidget_->setWindowFlags(Qt::Popup);
    listWidget_->setFocusPolicy(Qt::NoFocus);
    listWidget_->setAttribute(Qt::WA_DeleteOnClose, false);

    connect(listWidget_, &QListWidget::itemClicked, this,
        &ComboBoxEx::itemSelected);
    connect(listWidget_, &PopupListWidget::enterKeyPressed, this,
        &ComboBoxEx::handleEnterKeyPressed);

    setupEventFilter();
}

ComboBoxEx::~ComboBoxEx()
{
    if (listWidget_) {
        listWidget_->removeEventFilter(this);
        listWidget_->deleteLater();
    }
}

void ComboBoxEx::setItems(const QStringList& items)
{
    items_ = items;
    itemEnabled_.clear();

    for (int i = 0; i < items_.size(); ++i) {
        itemEnabled_[i] = true;
    }

    listWidget_->clear();
    listWidget_->addItems(items_);
}

void ComboBoxEx::addItem(const QString& item)
{
    items_.append(item);
    itemEnabled_[items_.size() - 1] = true;
    filterList(text());
}

void ComboBoxEx::setCurrentIndex(int index)
{
    if (index < -1 || index >= items_.size()) {
        return;
    }

    if (index != -1 && !itemEnabled_.value(index, true)) {
        return;
    }

    if (index == -1) {
        currentIndex_ = -1;
        setText(QString());
    } else {
        currentIndex_ = index;
        setText(items_[index]);
    }

    Q_EMIT currentIndexChanged(currentIndex_);
}

void ComboBoxEx::setItemEnabled(int index, bool enabled)
{
    if (index < 0 || index >= items_.size()) {
        return;
    }

    itemEnabled_[index] = enabled;

    if (listWidget_->isVisible()) {
        filterList(text());
    }
}

void ComboBoxEx::mousePressEvent(QMouseEvent* event)
{
    QLineEdit::mousePressEvent(event);
    showPopup();
}

void ComboBoxEx::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Down:
    case Qt::Key_Up:
        showPopup();
        if (listWidget_->count() > 0) {
            listWidget_->setCurrentRow(
                listWidget_->currentRow() < 0 ? 0 : listWidget_->currentRow());
            listWidget_->setFocus();
        }
        event->accept();
        break;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (listWidget_->isVisible() && listWidget_->currentItem()) {
            handleEnterKeyPressed(listWidget_->currentItem());
            event->accept();
        } else {
            QLineEdit::keyPressEvent(event);
        }
        break;

    default:
        QLineEdit::keyPressEvent(event);
        break;
    }
}

bool ComboBoxEx::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (listWidget_->isVisible()) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);

            QPoint globalPos;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            globalPos = mouseEvent->globalPosition().toPoint();
#else
            globalPos = mouseEvent->globalPos();
#endif

            const QRect popupRect = listWidget_->geometry();
            const QRect lineEditRect = QRect(mapToGlobal(QPoint(0, 0)), size());

            if (!popupRect.contains(globalPos)
                && !lineEditRect.contains(globalPos)) {
                hidePopup();
                return true;
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

void ComboBoxEx::itemSelected(QListWidgetItem* item)
{
    if (!item || !(item->flags() & Qt::ItemIsEnabled))
        return;

    int originalIndex = item->data(Qt::UserRole).toInt();
    currentIndex_ = originalIndex;
    setText(item->text());
    Q_EMIT currentIndexChanged(currentIndex_);
    hidePopup();
}

void ComboBoxEx::filterList(const QString& text)
{
    listWidget_->clear();

    for (int i = 0; i < items_.size(); ++i) {
        const QString& item = items_[i];
        if (item.contains(text, Qt::CaseInsensitive)) {
            QListWidgetItem* listItem = new QListWidgetItem(item, listWidget_);
            if (!itemEnabled_.value(i, true)) {
                listItem->setFlags(listItem->flags() & ~Qt::ItemIsEnabled);
                listItem->setForeground(Qt::gray);
            }
            listItem->setData(Qt::UserRole, i);
        }
    }

    /*for (const QString& item : items_) {
        if (item.contains(text, Qt::CaseInsensitive)) {
            listWidget_->addItem(item);
        }
    }*/

    if (listWidget_->count() > 0) {
        showPopup();
    } else {
        hidePopup();
    }
}

void ComboBoxEx::handleEnterKeyPressed(QListWidgetItem* item)
{
    itemSelected(item);
}

void ComboBoxEx::showPopup()
{
    if (listWidget_->count() == 0)
        return;

    QPoint pos = mapToGlobal(QPoint(0, height() + 3));
    listWidget_->move(pos);
    listWidget_->resize(width(), 150);

    if (listWidget_->currentRow() < 0 && listWidget_->count() > 0) {
        listWidget_->setCurrentRow(0);
    }

    if (currentIndex_ >= 0 && currentIndex_ < items_.size()) {
        for (int i = 0; i < listWidget_->count(); ++i) {
            if (listWidget_->item(i)->text() == items_[currentIndex_]) {
                listWidget_->setCurrentRow(i);
                break;
            }
        }
    }

    listWidget_->show();
}

void ComboBoxEx::hidePopup()
{
    listWidget_->hide();
    setFocus();
}

void ComboBoxEx::setupEventFilter()
{
    listWidget_->installEventFilter(this);
}
