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

#include "PlainTextEditEx.h"

namespace Daitengu::Components {

PlainTextEditEx::PlainTextEditEx(QWidget* parent)
    : QPlainTextEdit(parent)
    , maxLength_(160)
{
    setObjectName("plainTextEdit");
    setContextMenuPolicy(Qt::NoContextMenu);

    connect(this, &QPlainTextEdit::textChanged, this,
        &PlainTextEditEx::checkLength);
}

void PlainTextEditEx::checkLength()
{
    QString currentText = toPlainText();
    if (currentText.length() > maxLength_) {
        int pos = textCursor().position();
        setPlainText(currentText.left(maxLength_));
        QTextCursor cursor = textCursor();
        cursor.setPosition(pos > maxLength_ ? maxLength_ : pos);
        setTextCursor(cursor);
    }
}

}
