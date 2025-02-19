#include "TxListWidget.h"

namespace Daitengu::Components {

TxListWidget::TxListWidget(QWidget* parent)
    : QListWidget(parent)
{
    setIconSize(QSize(48, 48));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    setContextMenuPolicy(Qt::CustomContextMenu);
}

TxListWidget::~TxListWidget()
{
}

}
