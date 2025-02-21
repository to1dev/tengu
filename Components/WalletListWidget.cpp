#include "WalletListWidget.h"

namespace Daitengu::Components {

WalletListWidget::WalletListWidget(QWidget* parent)
    : QListWidget(parent)
{
    setObjectName(WALLET_OBJECT_NAME);
    setViewMode(QListView::IconMode);
    setIconSize(QSize(WALLET_ICON_SIZE, WALLET_ICON_SIZE));
    setResizeMode(QListView::Adjust);
    setSpacing(WALLET_SPACING_SIZE);
    verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    setDragEnabled(false);
    setContextMenuPolicy(Qt::CustomContextMenu);
}

}
