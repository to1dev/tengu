#include "FlowLayout.h"

namespace Daitengu::Components {

FlowLayout::FlowLayout(QGraphicsLayoutItem* parent)
    : QGraphicsLayout(parent)
{
    auto sp = sizePolicy();
    sp.setHeightForWidth(true);
    setSizePolicy(sp);
}

void FlowLayout::insertItem(int index, QGraphicsLayoutItem* item)
{
    item->setParentLayoutItem(this);
    if (index < 0 || index > items_.count()) {
        index = items_.count();
    }
    items_.insert(index, item);
    invalidate();
}

void FlowLayout::setSpacing(Qt::Orientations o, qreal spacing)
{
    if (o & Qt::Horizontal) {
        spacing_[0] = spacing;
    }

    if (o & Qt::Vertical) {
        spacing_[1] = spacing;
    }
}

qreal FlowLayout::spacing(Qt::Orientation o) const
{
    return spacing_[static_cast<std::size_t>(o) - 1];
}

void FlowLayout::setGeometry(const QRectF& geom)
{
    QGraphicsLayout::setGeometry(geom);
    doLayout(geom, true);
}

int FlowLayout::count() const
{
    return items_.count();
}

QGraphicsLayoutItem* FlowLayout::itemAt(int index) const
{
    return items_.value(index);
}

void FlowLayout::removeAt(int index)
{
    items_.removeAt(index);
    invalidate();
}

QSizeF FlowLayout::sizeHint(Qt::SizeHint which, const QSizeF& constraint) const
{
    QSizeF sh = constraint;
    switch (which) {
    case Qt::PreferredSize:
        sh = prefSize();
        break;
    case Qt::MinimumSize:
        sh = minSize(constraint);
        break;
    case Qt::MaximumSize:
        sh = maxSize();
        break;
    default:
        break;
    }
    return sh;
}

qreal FlowLayout::doLayout(const QRectF& geom, bool applyNewGeometry) const
{
    qreal left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    const qreal maxw = geom.width() - left - right;

    qreal x = 0;
    qreal y = 0;
    qreal maxRowHeight = 0;
    QSizeF pref;
    for (auto* item : items_) {
        pref = item->effectiveSizeHint(Qt::PreferredSize);
        maxRowHeight = qMax(maxRowHeight, pref.height());

        qreal next_x = x + pref.width();
        if (next_x > maxw) {
            if (qFuzzyIsNull(x)) {
                pref.setWidth(maxw);
            } else {
                x = 0;
                next_x = pref.width();
            }
            y += maxRowHeight + spacing(Qt::Vertical);
            maxRowHeight = 0;
        }

        if (applyNewGeometry)
            item->setGeometry(QRectF(QPointF(left + x, top + y), pref));
        x = next_x + spacing(Qt::Horizontal);
    }
    maxRowHeight = qMax(maxRowHeight, pref.height());
    return top + y + maxRowHeight + bottom;
}

QSizeF FlowLayout::minSize(const QSizeF& constraint) const
{
    QSizeF size(0, 0);
    qreal left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    if (constraint.width() >= 0) {
        const qreal height = doLayout(QRectF(QPointF(0, 0), constraint), false);
        size = QSizeF(constraint.width(), height);
    } else if (constraint.height() >= 0) {
    } else {
        for (const auto* item : qAsConst(items_))
            size = size.expandedTo(item->effectiveSizeHint(Qt::MinimumSize));
        size += QSizeF(left + right, top + bottom);
    }
    return size;
}

QSizeF FlowLayout::prefSize() const
{
    qreal left, right;
    getContentsMargins(&left, nullptr, &right, nullptr);

    qreal maxh = 0;
    qreal totalWidth = 0;
    for (const auto* item : qAsConst(items_)) {
        if (totalWidth > 0)
            totalWidth += spacing(Qt::Horizontal);
        QSizeF pref = item->effectiveSizeHint(Qt::PreferredSize);
        totalWidth += pref.width();
        maxh = qMax(maxh, pref.height());
    }
    maxh += spacing(Qt::Vertical);

    constexpr qreal goldenAspectRatio = 1.61803399;
    qreal w = qSqrt(totalWidth * maxh * goldenAspectRatio) + left + right;

    return minSize(QSizeF(w, -1));
}

QSizeF FlowLayout::maxSize() const
{
    qreal totalWidth = 0;
    qreal totalHeight = 0;
    for (const auto* item : qAsConst(items_)) {
        if (totalWidth > 0)
            totalWidth += spacing(Qt::Horizontal);
        if (totalHeight > 0)
            totalHeight += spacing(Qt::Vertical);
        QSizeF pref = item->effectiveSizeHint(Qt::PreferredSize);
        totalWidth += pref.width();
        totalHeight += pref.height();
    }

    qreal left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    return QSizeF(left + totalWidth + right, top + totalHeight + bottom);
}

}
