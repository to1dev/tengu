#ifndef GEOMETRYUTILS_H
#define GEOMETRYUTILS_H

#include <algorithm>

#include <QRect>
#include <QScreen>

#include "../Core/Types.h"

namespace Daitengu::Layouts {
namespace GeometryUtils {

    inline QRect getScreenGeometry(QScreen* screen)
    {
        return screen ? screen->geometry() : QRect();
    }

    inline QRect getAvailableScreenGeometry(QScreen* screen)
    {
        return screen ? screen->availableGeometry() : QRect();
    }

    inline QRect getCenteredRect(const QRect& rect, const QRect& container)
    {
        int x = container.x() + (container.width() - rect.width()) / 2;
        int y = container.y() + (container.height() - rect.height()) / 2;
        return QRect(x, y, rect.width(), rect.height());
    }

    QRect calculateGridRect(const GridCell& cell, const QRect& totalArea,
        int rows, int cols, int hGap = 0, int vGap = 0);

    inline QRect calculateGridAreaRect(const GridArea& area,
        const QRect& totalArea, int rows, int cols, int hGap = 0, int vGap = 0)
    {
        return calculateGridRect(
            area.toCell(), totalArea, rows, cols, hGap, vGap);
    }

    inline QRect scaleRect(
        const QRect& rect, double widthRatio, double heightRatio)
    {
        int newWidth = static_cast<int>(rect.width() * widthRatio);
        int newHeight = static_cast<int>(rect.height() * heightRatio);
        return QRect(rect.x(), rect.y(), newWidth, newHeight);
    }

    inline QRect insertRect(
        const QRect& rect, int left, int top, int right, int bottom)
    {
        return QRect(rect.x() + left, rect.y() + top,
            rect.width() - (left + right), rect.height() - (top + bottom));
    }

    inline QRect outsetRect(
        const QRect& rect, int left, int top, int right, int bottom)
    {
        return QRect(rect.x() - left, rect.y() - top,
            rect.width() + (left + right), rect.height() + (top + bottom));
    }

    inline QRect expandRect(const QRect& rect, double ratio)
    {
        int deltaWidth = static_cast<int>(rect.width() * (ratio - 1.0));
        int deltaHeight = static_cast<int>(rect.height() * (ratio - 1.0));
        return outsetRect(rect, deltaWidth / 2, deltaHeight / 2, deltaWidth / 2,
            deltaHeight / 2);
    }

    inline QRect shrinkRect(const QRect& rect, double ratio)
    {
        return expandRect(rect, 1.0 / ratio);
    }

    QSize constrainSize(
        const QSize& size, const QSize& minSize, const QSize& maxSize);
    QRect constrainRect(const QRect& rect, const QRect& bounds);

    inline QRect maintainAspectRatio(const QRect& rect, double aspectRatio)
    {
        if (aspectRatio <= 0)
            return rect;
        QRect result = rect;
        double currentRatio = static_cast<double>(rect.width()) / rect.height();
        if (currentRatio > aspectRatio) {
            int newWidth = static_cast<int>(rect.height() * aspectRatio);
            int deltaWidth = rect.width() - newWidth;
            result.setX(result.x() + deltaWidth / 2);
            result.setWidth(newWidth);
        } else if (currentRatio < aspectRatio) {
            int newHeight = static_cast<int>(rect.width() / aspectRatio);
            int deltaHeight = rect.height() - newHeight;
            result.setY(result.y() + deltaHeight / 2);
            result.setHeight(newHeight);
        }
        return result;
    }
}

}
#endif // GEOMETRYUTILS_H
