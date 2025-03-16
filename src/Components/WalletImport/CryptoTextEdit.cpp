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

#include "CryptoTextEdit.h"

namespace Daitengu::Components {

CryptoTextEdit::CryptoTextEdit(QWidget* parent)
    : QPlainTextEdit(parent)
{
    setObjectName(PLAINTEXTEDIT_NAME);
    setPlaceholderText(PLACEHOLDER_TEXT);

    connect(this, &QPlainTextEdit::textChanged, this, [this]() {
        QTimer::singleShot(300, this, &CryptoTextEdit::analyzeCurrentText);
    });
}

void CryptoTextEdit::insertFromMimeData(const QMimeData* source)
{
}

void CryptoTextEdit::keyReleaseEvent(QKeyEvent* e)
{
}

void CryptoTextEdit::focusOutEvent(QFocusEvent* e)
{
}

void CryptoTextEdit::analyzeCurrentText()
{
}

ContentInfo CryptoTextEdit::detectContent(const QString& text)
{
    return ContentInfo();
}

bool CryptoTextEdit::isMnemonic(const QString& text)
{
    return false;
}

bool CryptoTextEdit::isBitcoinAddress(const QString& text)
{
    return false;
}

bool CryptoTextEdit::isEthereumAddress(const QString& text)
{
    return false;
}

bool CryptoTextEdit::isSolanaAddress(const QString& text)
{
    return false;
}

bool CryptoTextEdit::isBitcoinPrivateKey(const QString& text)
{
    return false;
}

bool CryptoTextEdit::isEthereumPrivateKey(const QString& text)
{
    return false;
}

bool CryptoTextEdit::isSolanaPrivateKey(const QString& text)
{
    return false;
}

bool CryptoTextEdit::isWIFPrivateKey(const QString& text)
{
    return false;
}

bool CryptoTextEdit::isHexString(const QString& text, int expectedLength)
{
    return false;
}

bool CryptoTextEdit::validateChecksum(const std::vector<unsigned char>& data)
{
    return false;
}
}
