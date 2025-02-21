#ifndef WALLETLISTWIDGET_H
#define WALLETLISTWIDGET_H

#include <QListWidget>
#include <QPainter>
#include <QScrollBar>
#include <QStyledItemDelegate>

namespace Daitengu::Components {

inline constexpr char WALLET_OBJECT_NAME[] = "listWidgetWallet";

inline constexpr int WALLET_ICON_SIZE = 64;
inline constexpr int WALLET_SPACING_SIZE = 12;

class WalletListWidget : public QListWidget {
public:
    explicit WalletListWidget(QWidget* parent = nullptr);
};

}
#endif // WALLETLISTWIDGET_H
