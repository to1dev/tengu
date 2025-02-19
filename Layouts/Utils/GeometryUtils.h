#ifndef GEOMETRYUTILS_H
#define GEOMETRYUTILS_H

#include <QRect>
#include <QScreen>

#include "../Core/Types.h"

namespace Daitengu::Layouts {
namespace GeometryUtils {

    QRect getScreenGeometry(QScreen* screen);
    QRect getAvailableScreenGeometry(QScreen* screen);
    QRect getCenteredRect(const QRect& rect, const QRect& container);

    QRect calculateGridRect(const GridCell& cell, const QRect& totalArea,
        int rows, int cols, int hGap = 0, int vGap = 0);
    QRect calculateGridAreaRect(const GridArea& area, const QRect& totalArea,
        int rows, int cols, int hGap = 0, int vGap = 0);

    QRect scaleRect(const QRect& rect, double widthRatio, double heightRatio);
    QRect insertRect(
        const QRect& rect, int left, int top, int right, int bottom);
    QRect outsetRect(
        const QRect& rect, int left, int top, int right, int bottom);
    QRect expandRect(const QRect& rect, double ratio);
    QRect shrinkRect(const QRect& rect, double ratio);

    QSize constrainSize(
        const QSize& size, const QSize& minSize, const QSize& maxSize);
    QRect constrainRect(const QRect& rect, const QRect& bounds);
    QRect maintainAspectRatio(const QRect& rect, double aspectRatio);
}

}
#endif // GEOMETRYUTILS_H
