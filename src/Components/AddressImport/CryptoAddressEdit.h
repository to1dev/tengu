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

#include <QMimeData>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QSyntaxHighlighter>

#include "Wallets/Core/BitcoinWallet.h"
#include "Wallets/Core/EthereumWallet.h"
#include "Wallets/Core/SolanaWallet.h"
#include "Wallets/Core/Types.h"

using namespace Daitengu::Wallets;

namespace Daitengu::Components {

class CryptoAddressHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit CryptoAddressHighlighter(QTextDocument* parent = nullptr);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightRule> highlightRules_;

    friend class CryptoAddressEdit;
};

class CryptoAddressEdit : public QPlainTextEdit {
    Q_OBJECT

    const QString PLACEHOLDER_TEXT
        = QObject::tr("粘贴或输入以回车结尾的地址列表...");

public:
    explicit CryptoAddressEdit(QWidget* parent = nullptr);
    ~CryptoAddressEdit();

    void setChainType(ChainType type);

    ChainType getChainType() const
    {
        return currentChain_;
    }

    QStringList getValidAddresses() const;

    bool isValidAddress(const QString& address) const;

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private Q_SLOTS:
    void checkAndFormatAddresses();

private:
    CryptoAddressHighlighter* highlighter_;
    ChainType currentChain_ { ChainType::UNKNOWN };

    QRegularExpression btcRegex_ {
        "^(1[1-9A-HJ-NP-Za-km-z]{26,35}|3[1-9A-HJ-NP-Za-km-z]{26,35}|"
        "(bc|tb)1[qpzry9x8gf2tvdw0s3jn54khce6mua7l]{8,87})$"
    };
    QRegularExpression ethRegex_ { "^0x[0-9a-fA-F]{40}$" };
    QRegularExpression solRegex_ { "^[1-9A-HJ-NP-Za-km-z]{32,44}$" };

    void setupHighlighter();
    bool validateChainConsistency(const QStringList& addresses);
    void clean(QStringList& addresses);
    void removeInvalidAddresses();
};
}
