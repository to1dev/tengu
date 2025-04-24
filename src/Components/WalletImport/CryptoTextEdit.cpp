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
    setContextMenuPolicy(Qt::NoContextMenu);

    connect(this, &QPlainTextEdit::textChanged, this, [this]() {
        QTimer::singleShot(300, this, &CryptoTextEdit::analyzeCurrentText);
    });
}

void CryptoTextEdit::insertFromMimeData(const QMimeData* source)
{
    if (source->hasText()) {
        QString newText = filterText(source->text().simplified());
        QPlainTextEdit::insertPlainText(newText);
        lastDetectedInfo_ = ContentInfo();
        // analyzeCurrentText();
        QTimer::singleShot(10, this, &CryptoTextEdit::analyzeCurrentText);
    }
}

void CryptoTextEdit::keyPressEvent(QKeyEvent* e)
{
    Qt::Key key = Qt::Key(e->key());
    if (std::find(DefaultKeys.begin(), DefaultKeys.end(), key)
        != DefaultKeys.end()) {
        QString currentText = this->toPlainText() + e->text();
        QStringList words
            = currentText.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (words.count() >= MAX_WORDS) {
            e->ignore();
            return;
        }
    }

    QPlainTextEdit::keyPressEvent(e);
}

void CryptoTextEdit::focusOutEvent(QFocusEvent* e)
{
    QPlainTextEdit::focusOutEvent(e);
    setPlainText(toPlainText().simplified());
    analyzeCurrentText();
}

void CryptoTextEdit::analyzeCurrentText()
{
    QString currentText = toPlainText();
    if (currentText.length() > MAX_LENGTH) {
        setPlainText(currentText.left(MAX_LENGTH));
    }

    QString text = toPlainText().trimmed();

    if (text == lastAnalyzedText_) {
        return;
    }

    lastAnalyzedText_ = text;

    if (text.isEmpty()) {
        if (lastDetectedInfo_.type != WalletType::Unknown) {
            lastDetectedInfo_ = ContentInfo();
            Q_EMIT contentDetected(lastDetectedInfo_);
        }
        return;
    }

    ContentInfo info = detectContent(text);

    if (!info.equals(lastDetectedInfo_)) {
        lastDetectedInfo_ = info;
        Q_EMIT contentDetected(info);
    }
}

QString CryptoTextEdit::filterText(const QString& text)
{
    QString currentText = this->toPlainText();
    QStringList currentWords
        = currentText.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    int currentWordCount = currentWords.count();

    QStringList newWords
        = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    int newWordCount = newWords.count();

    if (currentWordCount + newWordCount > MAX_WORDS) {
        int wordsToAdd = MAX_WORDS - currentWordCount;
        if (wordsToAdd > 0) {
            return newWords.mid(0, wordsToAdd).join(" ");
        }
        return QString();
    }
    return text;
}

ContentInfo CryptoTextEdit::detectContent(const QString& text)
{
    ContentInfo info;
    info.content = text;

    try {
        if (isMnemonic(text)) {
            info.type = WalletType::Mnemonic;
            return info;
        }

        if (text.startsWith("0x") && isEthereumAddress(text)) {
            info.type = WalletType::Address;
            info.chain = ChainType::ETHEREUM;
            return info;
        }

        if (std::any_of(btcPrefixes.begin(), btcPrefixes.end(),
                [&](const char* prefix) { return text.startsWith(prefix); })
            && isBitcoinAddress(text)) {
            info.type = WalletType::Address;
            info.chain = ChainType::BITCOIN;
            return info;
        }

        if (std::any_of(wifPrefixes.begin(), wifPrefixes.end(),
                [&](const char* prefix) { return text.startsWith(prefix); })
            && isWIFPrivateKey(text)) {
            info.type = WalletType::Wif;
            info.chain = ChainType::BITCOIN;
            return info;
        }

        if (isSolanaAddress(text)) {
            info.type = WalletType::Address;
            info.chain = ChainType::SOLANA;
            return info;
        }

        if (isEthereumPrivateKey(text)) {
            info.type = WalletType::Priv;
            info.chain = ChainType::ETHEREUM;
            return info;
        }

        /*if (isBitcoinPrivateKey(text)) {
            info.type = WalletType::Priv;
            info.chain = ChainType::BITCOIN;
            return info;
        }*/

        if (isSolanaPrivateKey(text)) {
            info.type = WalletType::Priv;
            info.chain = ChainType::SOLANA;
            return info;
        }
    } catch (const std::exception& e) {
        qDebug() << "Exception during content detection: " << e.what();
    }

    info.type = WalletType::Unknown;
    info.chain = ChainType::UNKNOWN;
    info.network = NetworkType::UNKNOWN;
    return info;
}

bool CryptoTextEdit::isMnemonic(const QString& text)
{
    try {
        const QStringList words = text.split(' ', Qt::SkipEmptyParts);

        if (words.size() != 12 && words.size() != 24) {
            return false;
        }

        return BaseMnemonic::check(text.toStdString());
    } catch (const std::exception& e) {
        qDebug() << "Failed to check mnemonic: " << e.what();
    }

    return false;
}

bool CryptoTextEdit::isBitcoinAddress(const QString& text)
{
    if (!bitcoinRegex_.match(text).hasMatch()) {
        return false;
    }

    return BitcoinWallet::isValid(text.toStdString());
}

bool CryptoTextEdit::isEthereumAddress(const QString& text)
{
    if (!ethRegex_.match(text).hasMatch()) {
        return false;
    }

    return EthereumWallet::isValid(text.toStdString());
}

bool CryptoTextEdit::isSolanaAddress(const QString& text)
{
    if (!base58Regex_.match(text).hasMatch()) {
        return false;
    }

    return SolanaWallet::isValid(text.toStdString());
}

bool CryptoTextEdit::isBitcoinPrivateKey(const QString& text)
{
    return text.length() == 64 && isHexString(text, 64);
}

bool CryptoTextEdit::isEthereumPrivateKey(const QString& text)
{
    return text.length() == 64 && isHexString(text, 64);
}

bool CryptoTextEdit::isSolanaPrivateKey(const QString& text)
{
    try {
        std::vector<unsigned char> decoded;
        if (!DecodeBase58(text.toStdString(), decoded, 80)) {
            return false;
        }

        return decoded.size() == 64;
    } catch (const std::exception& e) {
        return false;
    }
}

bool CryptoTextEdit::isWIFPrivateKey(const QString& text)
{
    if (!wifRegex_.match(text).hasMatch()) {
        return false;
    }

    try {
        std::vector<unsigned char> decoded;
        if (!DecodeBase58(text.toStdString(), decoded, 40)) {
            return false;
        }

        if (decoded.size() != 38) {
            return false;
        }

        std::vector<unsigned char> payload(
            decoded.begin(), decoded.begin() + 34);

        if (payload[0] != 0x80) {
            return false;
        }

        if (payload[33] != 0x01) {
            return false;
        }

        return validateChecksum(decoded);
    } catch (const std::exception& e) {
        qDebug() << "Failed to check WIF private key: " << e.what();
    }

    return false;
}

bool CryptoTextEdit::isHexString(const QString& text, int expectedLength)
{
    if (!hexRegex_.match(text).hasMatch()) {
        return false;
    }

    if (expectedLength > 0 && text.length() != expectedLength) {
        return false;
    }

    return true;
}

bool CryptoTextEdit::validateChecksum(const std::vector<unsigned char>& data)
{
    const size_t payloadSize = data.size() - 4;
    const std::vector<unsigned char> payload(
        data.begin(), data.begin() + payloadSize);

    unsigned char hash1[32], hash2[32];
    sha256_Raw(payload.data(), payload.size(), hash1);
    sha256_Raw(hash1, 32, hash2);

    return (hash2[0] == data[payloadSize] && hash2[1] == data[payloadSize + 1]
        && hash2[2] == data[payloadSize + 2]
        && hash2[3] == data[payloadSize + 3]);
}
}
