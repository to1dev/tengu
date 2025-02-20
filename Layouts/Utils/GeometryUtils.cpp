#include "GeometryUtils.h"

namespace Daitengu::Layouts {

namespace GeometryUtils {

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
        int width = static_cast<int>(
            cell.colSpan * cellWidth + (cell.colSpan - 1) * hGap);
        int height = static_cast<int>(
            cell.rowSpan * cellHeight + (cell.rowSpan - 1) * vGap);

        return QRect(x, y, width, height);
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

}
}
