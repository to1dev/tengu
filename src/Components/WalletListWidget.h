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
#include <string_view>
#include <unordered_map>
#include <vector>

#include <QAbstractListModel>
#include <QListView>
#include <QPainter>
#include <QScrollBar>
#include <QStyledItemDelegate>
#include <QSvgRenderer>

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
inline constexpr char BADGE_ICON[] = ":/Badges/import%1.svg";

inline constexpr char WALLET_ICON_BITCOIN[] = ":/List/bitcoin.svg";
inline constexpr char WALLET_ICON_BITCOIN_IMPORT[] = ":/List/bitcoini.svg";
inline constexpr char WALLET_ICON_ETHEREUM[] = ":/List/ethereum.svg";
inline constexpr char WALLET_ICON_ETHEREUM_IMPORT[] = ":/List/ethereumi.svg";
inline constexpr char WALLET_ICON_SOLANA[] = ":/List/solana.svg";
inline constexpr char WALLET_ICON_SOLANA_IMPORT[] = ":/List/solanai.svg";
inline constexpr char WALLET_ICON_TRON[] = ":/List/tron.svg";
inline constexpr char WALLET_ICON_TRON_IMPORT[] = ":/List/troni.svg";
inline constexpr char WALLET_ICON_BNB[] = ":/List/bnb.svg";
inline constexpr char WALLET_ICON_BNB_IMPORT[] = ":/List/bnbi.svg";

inline constexpr char WALLET_OBJECT_NAME[] = "listWidgetWallet";

inline constexpr int WALLET_ICON_SIZE = 64;
inline constexpr int WALLET_SPACING_SIZE = 12;

struct IconItem {
    int chain;
    std::string_view icon;
};

struct IconItems {
    int type;
    std::array<IconItem, 3> items;
};

inline constexpr std::array<IconItems, 2> WalletListIcons
    = { { {
              0,
              { {
                  { 0, WALLET_ICON_BITCOIN },
                  { 1, WALLET_ICON_ETHEREUM },
                  { 2, WALLET_ICON_SOLANA },
              } },
          },
        {
            1,
            { {
                { 0, WALLET_ICON_BITCOIN_IMPORT },
                { 1, WALLET_ICON_ETHEREUM_IMPORT },
                { 2, WALLET_ICON_SOLANA_IMPORT },
            } },
        } } };

inline const auto WalletTypeText = std::unordered_map<int, std::string_view> {
    { 1, "M" },
    { 2, "K" },
    { 3, "W" },
    { 4, "A" },
};

class BadgeItemDelegate : public QStyledItemDelegate {
public:
    explicit BadgeItemDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
};

class WalletTypeBadgeDelegate : public QStyledItemDelegate {
public:
    explicit WalletTypeBadgeDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
};

class WalletListModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum ItemData {
        Id = Qt::UserRole + 100,
        Type,
        GroupType,
        ChainType,
        NetworkType,
        Index,
        Hash,
        Name,
        Mnemonic,
    };

    explicit WalletListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(
        const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void add(const Wallet& wallet);
    void load(const std::vector<Wallet>& wallets);
    void update(const Wallet& wallet);
    void remove(const QList<int>& rows);

private:
    std::vector<Wallet> wallets_;
};

namespace {
    static const QIcon& getCachedIcon(int group, int chain)
    {
        static bool initialized = false;
        static QIcon iconCache[2][3];
        if (!initialized) {
            for (int g = 0; g < 2; ++g) {
                for (int c = 0; c < 3; ++c) {
                    iconCache[g][c] = QIcon(QString::fromUtf8(
                        WalletListIcons[g].items[c].icon.data(),
                        static_cast<int>(
                            WalletListIcons[g].items[c].icon.size())));
                }
            }
            initialized = true;
        }
        return iconCache[group][chain];
    }
} // end anonymous namespace

class WalletListView : public QListView {
    Q_OBJECT

public:
    explicit WalletListView(QWidget* parent = nullptr);

    void load(const std::vector<Wallet>& wallets);
    void add(const Wallet& wallet);
    void update(const Wallet& wallet);
    void remove(const QList<int>& rows);

    WalletListModel* model() const
    {
        return model_;
    }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    WalletListModel* model_;
};

}
#endif // WALLETLISTWIDGET_H
