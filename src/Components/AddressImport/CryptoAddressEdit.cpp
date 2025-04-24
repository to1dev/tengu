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

#include "CryptoAddressEdit.h"

#include <QDebug>
#include <QScrollBar>

namespace Daitengu::Components {

CryptoAddressHighlighter::CryptoAddressHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
}

void CryptoAddressHighlighter::highlightBlock(const QString& text)
{
    for (const HighlightRule& rule : highlightRules_) {
        QRegularExpressionMatchIterator matchIterator
            = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(
                match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

CryptoAddressEdit::CryptoAddressEdit(QWidget* parent)
    : QPlainTextEdit(parent)
{
    setAcceptDrops(true);
    setPlaceholderText(PLACEHOLDER_TEXT);
    setContextMenuPolicy(Qt::NoContextMenu);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    highlighter_ = new CryptoAddressHighlighter(document());
    setupHighlighter();
}

CryptoAddressEdit::~CryptoAddressEdit() = default;

void CryptoAddressEdit::setChainType(ChainType type)
{
    currentChain_ = type;
    checkAndFormatAddresses();
}

QStringList CryptoAddressEdit::getValidAddresses() const
{
    QStringList validAddresses;
    QStringList lines = toPlainText().split("\n", Qt::SkipEmptyParts);

    for (QString address : lines) {
        address = address.trimmed();
        if (isValidAddress(address)) {
            validAddresses.append(address);
        }
    }
    return validAddresses;
}

bool CryptoAddressEdit::isValidAddress(const QString& address) const
{
    switch (currentChain_) {
    case ChainType::BITCOIN:
        return BitcoinWallet::isValid(address.toStdString());
    case ChainType::ETHEREUM:
        return EthereumWallet::isValid(address.toStdString());
    case ChainType::SOLANA:
        return SolanaWallet::isValid(address.toStdString());
    default:
        break;
    }

    return false;
}

void CryptoAddressEdit::keyPressEvent(QKeyEvent* event)
{
    QPlainTextEdit::keyPressEvent(event);
}

void CryptoAddressEdit::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();
    }
}

void CryptoAddressEdit::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasText()) {
        QString text = event->mimeData()->text();
        insertPlainText(text);
        event->acceptProposedAction();
    }
}

void CryptoAddressEdit::checkAndFormatAddresses()
{
    QString text = toPlainText().trimmed();

    QTextCursor cursor = textCursor();
    int position = cursor.position();

    highlighter_->rehighlight();

    QStringList lines = toPlainText().split("\n", Qt::SkipEmptyParts);
    QStringList addresses;
    for (const QString& line : lines) {
        addresses.append(line.trimmed());
    }

    clean(addresses);
    setPlainText(addresses.join("\n"));

    if (!validateChainConsistency(addresses)
        && currentChain_ != ChainType::UNKNOWN) {
        removeInvalidAddresses();
    }

    cursor.setPosition(qMin(position, toPlainText().length()));
    setTextCursor(cursor);
}

void CryptoAddressEdit::setupHighlighter()
{
    CryptoAddressHighlighter::HighlightRule rule;

    QTextCharFormat btcFormat;
    btcFormat.setForeground(QColor("#FFB74D"));
    btcFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    rule.pattern = btcRegex_;
    rule.format = btcFormat;
    highlighter_->highlightRules_.append(rule);

    QTextCharFormat ethFormat;
    ethFormat.setForeground(QColor("#64B5F6"));
    ethFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    rule.pattern = ethRegex_;
    rule.format = ethFormat;
    highlighter_->highlightRules_.append(rule);

    QTextCharFormat solFormat;
    solFormat.setForeground(QColor("#F06292"));
    solFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    rule.pattern = solRegex_;
    rule.format = solFormat;
    highlighter_->highlightRules_.append(rule);
}

bool CryptoAddressEdit::validateChainConsistency(const QStringList& addresses)
{
    if (currentChain_ == ChainType::UNKNOWN || addresses.isEmpty()) {
        return true;
    }

    for (const QString& address : addresses) {
        if (!isValidAddress(address)) {
            return false;
        }
    }
    return true;
}

void CryptoAddressEdit::clean(QStringList& addresses)
{
    QStringList result;
    QSet<QString> seenAddresses;
    QRegularExpression specialChars("[^a-zA-Z0-9]");

    for (auto& address : addresses) {
        QString cleaned = address.remove(specialChars);
        if (!cleaned.isEmpty() && !seenAddresses.contains(cleaned)) {
            seenAddresses.insert(cleaned);
            result.append(cleaned);
        }
    }

    addresses = result;
}

void CryptoAddressEdit::removeInvalidAddresses()
{
    QStringList lines = toPlainText().split("\n", Qt::SkipEmptyParts);
    QStringList validLines;

    for (QString line : lines) {
        line = line.trimmed();
        if (isValidAddress(line)) {
            validLines.append(line);
        }
    }

    setPlainText(validLines.join("\n"));
}

}
