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

#ifndef WALLETLISTWIDGET_H
#define WALLETLISTWIDGET_H

#include <array>

#include <QListWidget>
#include <QPainter>
#include <QScrollBar>
#include <QStyledItemDelegate>

#include "Databases/Database.h"

using namespace Daitengu::Databases;

namespace Daitengu::Components {

inline constexpr char WALLET_ICON_DEFAULT[] = ":/List/wallet.svg";
inline constexpr char WALLET_ICON_KEY[] = ":/List/key.svg";
inline constexpr char WALLET_ICON_MNEMONIC[] = ":/List/mnemonic.svg";
inline constexpr char WALLET_ICON_TRADE[] = ":/List/trade.svg";
inline constexpr char WALLET_ICON_SEARCH[] = ":/List/search.svg";
inline constexpr char WALLET_ICON_GOLD[] = ":/List/gold.svg";
inline constexpr char WALLET_ICON_COINS[] = ":/List/coins.svg";
inline constexpr char BADGE_ICON[] = ":/Badges/check.svg";

inline constexpr char WALLET_ICON_BITCOIN[] = ":/List/bitcoin.svg";
inline constexpr char WALLET_ICON_ETHEREUM[] = ":/List/ethereum.svg";
inline constexpr char WALLET_ICON_SOLANA[] = ":/List/solana.svg";
inline constexpr char WALLET_ICON_TRON[] = ":/List/tron.svg";
inline constexpr char WALLET_ICON_BNB[] = ":/List/bnb.svg";

inline constexpr char WALLET_OBJECT_NAME[] = "listWidgetWallet";

inline constexpr int WALLET_ICON_SIZE = 64;
inline constexpr int WALLET_SPACING_SIZE = 12;

inline constexpr std::array<std::pair<int, std::string_view>, 5> WalletListIcons
    = { {
        { 0, WALLET_ICON_BITCOIN },
        { 1, WALLET_ICON_ETHEREUM },
        { 2, WALLET_ICON_SOLANA },
        { 3, WALLET_ICON_TRON },
        { 4, WALLET_ICON_BNB },
    } };

class BadgeItemDelegate : public QStyledItemDelegate {
public:
    explicit BadgeItemDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
};

class ChainTypeBadgeDelegate : public QStyledItemDelegate {
public:
    explicit ChainTypeBadgeDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
};

class WalletListWidget : public QListWidget {
public:
    enum class ItemData {
        selected = Qt::UserRole + 100,
        id,
        type,
        groupType,
        chainType,
        networkType,
        index,
        hash,
        name,
        mnemonic,
        derivationPath,
    };

    explicit WalletListWidget(QWidget* parent = nullptr);
    bool focusChanged();

    void add(const Wallet& wallet, int index = 0);
    void load(const std::vector<Wallet>& wallets);
    void update(const Wallet& wallet);
    void purge();

    void setSelectedId(int newSelectedId);

private:
    int selectedId_ { -1 };
};

}
#endif // WALLETLISTWIDGET_H
