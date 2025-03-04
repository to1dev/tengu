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

#include <QAbstractItemView>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QScrollBar>
#include <QVBoxLayout>

class PopupListWidget : public QListWidget {
    Q_OBJECT

public:
    explicit PopupListWidget(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;

Q_SIGNALS:
    void enterKeyPressed(QListWidgetItem* currentItem);
};

class ComboBoxEx : public QLineEdit {
    Q_OBJECT

public:
    explicit ComboBoxEx(QWidget* parent = nullptr);
    ~ComboBoxEx() override;

    void setItems(const QStringList& items);
    void addItem(const QString& item);

    [[nodiscard]] int currentIndex() const
    {
        return currentIndex_;
    }

    void setCurrentIndex(int index);
    void setItemEnabled(int index, bool enabled);

Q_SIGNALS:
    void currentIndexChanged(int index);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private Q_SLOTS:
    void itemSelected(QListWidgetItem* item);
    void filterList(const QString& text);
    void handleEnterKeyPressed(QListWidgetItem* item);

private:
    void showPopup();
    void hidePopup();
    void setupEventFilter();

    PopupListWidget* listWidget_;
    QStringList items_;
    int currentIndex_;

    QMap<int, bool> itemEnabled_;
};
