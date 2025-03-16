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

#include <array>

#include <QDebug>
#include <QKeyEvent>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QTimer>

#include "Wallets/Core/BaseMnemonic.h"
#include "Wallets/Core/BitcoinWallet.h"
#include "Wallets/Core/EthereumWallet.h"
#include "Wallets/Core/SolanaWallet.h"

using namespace Daitengu::Wallets;

#include "Utils/Base58.hpp"

#include "ContentTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "izanagi/sha2.h"

#ifdef __cplusplus
}
#endif

namespace Daitengu::Components {

inline constexpr int MAX_WORDS = 24;
inline constexpr char PLAINTEXTEDIT_NAME[] = "plainTextEdit";

inline constexpr std::array<Qt::Key, 3> DefaultKeys = {
    Qt::Key_Space,
    Qt::Key_Return,
    Qt::Key_Tab,
};

inline constexpr std::array<const char*, 2> btcPrefixes = {
    "bc1",
    "tb1",
};

inline constexpr std::array<const char*, 3> wifPrefixes = {
    "5",
    "K",
    "L",
};

class CryptoTextEdit : public QPlainTextEdit {
    Q_OBJECT

    const QString PLACEHOLDER_TEXT
        = QObject::tr("粘贴或输入助记词、地址或私钥...");

public:
    explicit CryptoTextEdit(QWidget* parent = nullptr);
    ~CryptoTextEdit() override = default;

Q_SIGNALS:
    void contentDetected(const ContentInfo& info);

protected:
    void insertFromMimeData(const QMimeData* source) override;
    void keyPressEvent(QKeyEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;

private Q_SLOTS:
    void analyzeCurrentText();

private:
    QString filterText(const QString& text);
    ContentInfo detectContent(const QString& text);

    bool isMnemonic(const QString& text);
    bool isBitcoinAddress(const QString& text);
    bool isEthereumAddress(const QString& text);
    bool isSolanaAddress(const QString& text);
    bool isBitcoinPrivateKey(const QString& text);
    bool isEthereumPrivateKey(const QString& text);
    bool isSolanaPrivateKey(const QString& text);
    bool isWIFPrivateKey(const QString& text);

    bool isHexString(const QString& text, int expectedLength = -1);
    bool validateChecksum(const std::vector<unsigned char>& data);

    QString lastAnalyzedText_;
    ContentInfo lastDetectedInfo_;

    QRegularExpression hexRegex_ { "^[0-9a-fA-F]+$" };
    QRegularExpression bitcoinRegex_ {
        "^(bc|tb)1[023456789acdefghjklmnpqrstuvwxyz]{6,}$"
    };
    QRegularExpression ethRegex_ { "^0x[0-9a-fA-F]{40}$" };
    QRegularExpression base58Regex_ { "^[1-9A-HJ-NP-Za-km-z]{32,44}$" };
    QRegularExpression wifRegex_ { "^[5KL][1-9A-HJ-NP-Za-km-z]{50,52}$" };
};

}
