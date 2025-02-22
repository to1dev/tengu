#include "FlowLayout.h"

namespace Daitengu::Components {

FlowLayout::FlowLayout(QGraphicsLayoutItem* parent)
    : QGraphicsLayout(parent)
{
    auto sp = sizePolicy();
    sp.setHeightForWidth(true);
    setSizePolicy(sp);
}

void FlowLayout::addItem(QGraphicsLayoutItem* item)
{
    insertItem(-1, item);
}

void FlowLayout::insertItem(int index, QGraphicsLayoutItem* item)
{
}

void FlowLayout::setSpacing(Qt::Orientations o, qreal spacing)
{
}

qreal FlowLayout::spacing(Qt::Orientation o) const
{
}

void FlowLayout::setGeometry(const QRectF& geom)
{
}

int FlowLayout::count() const
{
}

QGraphicsLayoutItem* FlowLayout::itemAt(int index) const
{
}

void FlowLayout::removeAt(int index)
{
}

QSizeF FlowLayout::sizeHint(Qt::SizeHint which, const QSizeF& constraint) const
{
}

qreal FlowLayout::doLayout(const QRectF& geom, bool applyNewGeometry) const
{
}

QSizeF FlowLayout::minSize(const QSizeF& constraint) const
{
}

QSizeF FlowLayout::prefSize() const
{
}

QSizeF FlowLayout::maxSize() const
{
}

}
