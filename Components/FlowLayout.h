#ifndef FLOWLAYOUT_H
#define FLOWLAYOUT_H

#include <array>

#include <QGraphicsLayout>
#include <QRect>
#include <QRectF>
#include <QVector>
#include <QtMath>

namespace Daitengu::Components {

class FlowLayout : public QGraphicsLayout {
public:
    explicit FlowLayout(QGraphicsLayoutItem* parent = nullptr);
    inline void addItem(QGraphicsLayoutItem* item);
    void insertItem(int index, QGraphicsLayoutItem* item);
    void setSpacing(Qt::Orientations o, qreal spacing);
    qreal spacing(Qt::Orientation o) const;

    void setGeometry(const QRectF& geom) override;

    [[nodiscard]] int count() const override;
    QGraphicsLayoutItem* itemAt(int index) const override;
    void removeAt(int index) override;

protected:
    QSizeF sizeHint(
        Qt::SizeHint which, const QSizeF& constraint = QSizeF()) const override;

private:
    qreal doLayout(const QRectF& geom, bool applyNewGeometry) const;
    QSizeF minSize(const QSizeF& constraint) const;
    QSizeF prefSize() const;
    QSizeF maxSize() const;

    QVector<QGraphicsLayoutItem*> items_;
    std::array<qreal, 2> spacing_ { { 6, 6 } };
};

inline void FlowLayout::addItem(QGraphicsLayoutItem* item)
{
    insertItem(-1, item);
}

}
#endif // FLOWLAYOUT_H
