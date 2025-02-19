#include "GeometryUtils.h"

namespace Daitengu::Layouts {

namespace GeometryUtils {

    QRect getScreenGeometry(QScreen* screen)
    {
        return screen ? screen->geometry() : QRect();
    }

    QRect getAvailableScreenGeometry(QScreen* screen)
    {
        return screen ? screen->availableGeometry() : QRect();
    }

    QRect getCenteredRect(const QRect& rect, const QRect& container)
    {
        int x = container.x() + (container.width() - rect.width()) / 2;
        int y = container.y() + (container.height() - rect.height()) / 2;
        return QRect(x, y, rect.width(), rect.height());
    }

    QRect calculateGridRect(const GridCell& cell, const QRect& totalArea,
        int rows, int cols, int hGap, int vGap)
    {
        if (rows <= 0 || cols <= 0)
            return QRect();

        double cellWidth = (totalArea.width() - (cols - 1) * hGap)
            / static_cast<double>(cols);
        double cellHeight = (totalArea.height() - (rows - 1) * vGap)
            / static_cast<double>(rows);

        int x = totalArea.x() + cell.col * (cellWidth + hGap);
        int y = totalArea.y() + cell.row * (cellHeight + vGap);
        int width = cell.colSpan * cellWidth + (cell.colSpan - 1) * hGap;
        int height = cell.rowSpan * cellHeight + (cell.rowSpan - 1) * vGap;

        return QRect(x, y, width, height);
    }

    QRect calculateGridAreaRect(const GridArea& area, const QRect& totalArea,
        int rows, int cols, int hGap, int vGap)
    {
        return calculateGridRect(
            area.toCell(), totalArea, rows, cols, hGap, vGap);
    }

    QRect scaleRect(const QRect& rect, double widthRatio, double heightRatio)
    {
        int newWidth = static_cast<int>(rect.width() * widthRatio);
        int newHeight = static_cast<int>(rect.height() * heightRatio);
        return QRect(rect.x(), rect.y(), newWidth, newHeight);
    }

    QRect insertRect(
        const QRect& rect, int left, int top, int right, int bottom)
    {
        return QRect(rect.x() + left, rect.y() + top,
            rect.width() - (left + right), rect.height() - (top + bottom));
    }

    QRect outsetRect(
        const QRect& rect, int left, int top, int right, int bottom)
    {
        return QRect(rect.x() - left, rect.y() - top,
            rect.width() + (left + right), rect.height() + (top + bottom));
    }

    QRect expandRect(const QRect& rect, double ratio)
    {
        int deltaWidth = static_cast<int>(rect.width() * (ratio - 1.0));
        int deltaHeight = static_cast<int>(rect.height() * (ratio - 1.0));
        return outsetRect(rect, deltaWidth / 2, deltaHeight / 2, deltaWidth / 2,
            deltaHeight / 2);
    }

    QRect shrinkRect(const QRect& rect, double ratio)
    {
        return expandRect(rect, 1.0 / ratio);
    }

    QSize constrainSize(
        const QSize& size, const QSize& minSize, const QSize& maxSize)
    {
        return QSize(std::clamp(size.width(), minSize.width(), maxSize.width()),
            std::clamp(size.height(), minSize.height(), maxSize.height()));
    }

    QRect constrainRect(const QRect& rect, const QRect& bounds)
    {
        QRect result = rect;

        if (result.right() > bounds.right())
            result.moveRight(bounds.right());
        if (result.bottom() > bounds.bottom())
            result.moveBottom(bounds.bottom());
        if (result.left() < bounds.left())
            result.moveLeft(bounds.left());
        if (result.top() < bounds.top())
            result.moveTop(bounds.top());

        if (result.width() > bounds.width())
            result.setWidth(bounds.width());
        if (result.height() > bounds.height())
            result.setHeight(bounds.height());

        return result;
    }

    QRect maintainAspectRatio(const QRect& rect, double aspectRatio)
    {
        if (aspectRatio <= 0)
            return rect;

        QRect result = rect;
        double currentRatio = static_cast<double>(rect.width()) / rect.height();

        if (currentRatio > aspectRatio) {
            int newWidth = static_cast<int>(rect.height() * aspectRatio);
            int deltaWidth = rect.width() - newWidth;
            result.setLeft(result.left() + deltaWidth / 2);
            result.setWidth(newWidth);
        } else if (currentRatio < aspectRatio) {
            int newHeight = static_cast<int>(rect.width() / aspectRatio);
            int deltaHeight = rect.height() - newHeight;
            result.setTop(result.top() + deltaHeight / 2);
            result.setHeight(newHeight);
        }

        return result;
    }

}
}
