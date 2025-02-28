#include "WalletListWidget.h"

namespace Daitengu::Components {

BadgeItemDelegate::BadgeItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void BadgeItemDelegate::paint(QPainter* painter,
    const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    QVariant data
        = index.data(static_cast<int>(WalletListWidget::ItemData::selected));
    if (data.isValid() && data.toBool()) {
        QIcon badgeIcon(BADGE_ICON);
        QSize badgeSize(32, 32);
        // QPoint badgePosition(option.rect.topRight().x() - badgeSize.width(),
        // option.rect.topRight().y() - 6);
        QPoint badgePosition(
            option.rect.topLeft().x(), option.rect.topRight().y() - 6);
        QRect badgeRect(badgePosition, badgeSize);

        painter->save();
        badgeIcon.paint(painter, badgeRect);
        painter->restore();
    }
}

ChainTypeBadgeDelegate::ChainTypeBadgeDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void ChainTypeBadgeDelegate::paint(QPainter* painter,
    const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    QString chainType
        = index.data(static_cast<int>(WalletListWidget::ItemData::chainType))
              .toString();

    if (!chainType.isEmpty()) {
        QString badgeText = chainType.left(1).toLower();

        painter->setRenderHint(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::TextAntialiasing);
        painter->setRenderHint(QPainter::SmoothPixmapTransform);

        int badgeSize = qMin(option.rect.width() / 3, 24);
        int badgeX = option.rect.right() - badgeSize - 3;
        int badgeY = option.rect.top() + 3;
        QRect badgeRect(badgeX, badgeY, badgeSize, badgeSize);

        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(QColor(255, 0, 0)));
        painter->drawEllipse(badgeRect);

        painter->setPen(QPen(QColor(255, 255, 255)));
        QFont font = painter->font();
        font.setBold(true);
        painter->setFont(font);
        painter->drawText(badgeRect, Qt::AlignCenter, badgeText);
    }
}

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

    // setItemDelegate(new ChainTypeBadgeDelegate(this));
}

bool WalletListWidget::focusChanged()
{
    return currentItem() ? !currentItem()->isSelected() : false;
}

void WalletListWidget::add(const Wallet& wallet, int index)
{
    QListWidgetItem* item = new QListWidgetItem;
    QString name = QString::fromStdString(wallet.name);
    item->setText(name);
    item->setIcon(QIcon(
        QString::fromUtf8(WalletListIcons[wallet.chainType].second.data())));
    item->setToolTip(name);

    item->setData(static_cast<int>(ItemData::selected), false);
    item->setData(static_cast<int>(ItemData::id), wallet.id);
    item->setData(static_cast<int>(ItemData::index), index);
    item->setData(static_cast<int>(ItemData::type), wallet.type);
    item->setData(static_cast<int>(ItemData::groupId), wallet.groupId);
    item->setData(static_cast<int>(ItemData::chainType), wallet.chainType);
    item->setData(static_cast<int>(ItemData::networkType), wallet.networkType);
    item->setData(
        static_cast<int>(ItemData::hash), QString::fromStdString(wallet.hash));
    item->setData(static_cast<int>(ItemData::name), name);
    item->setData(static_cast<int>(ItemData::mnemonic),
        QString::fromStdString(wallet.mnemonic));

    addItem(item);

    if (selectedId_ == wallet.id) {
        item->setSelected(true);
        setCurrentRow(row(item));
        scrollToItem(item);
        // setFocus();
    }
}

void WalletListWidget::load(const std::vector<Wallet>& wallets)
{
    setUpdatesEnabled(false);

    int index = 0;
    for (const Wallet& wallet : wallets) {
        add(wallet, index++);
    }

    setUpdatesEnabled(true);
}

void WalletListWidget::update(const Wallet& wallet)
{
    QListWidgetItem* item = currentItem();
    QString name = QString::fromStdString(wallet.name);
    item->setText(name);
    item->setToolTip(name);
    item->setData(static_cast<int>(ItemData::name), name);
}

void WalletListWidget::purge()
{
    QList<QListWidgetItem*> items = selectedItems();
    if (!items.isEmpty()) {
        qDeleteAll(items);
    }
}

void WalletListWidget::setSelectedId(int newSelectedId)
{
    selectedId_ = newSelectedId;
}

}
