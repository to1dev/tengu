#ifndef TYPES_H
#define TYPES_H

#include <QEasingCurve>
#include <QRect>
#include <QString>

namespace Daitengu::Layouts {

struct GridCell {
    int row;
    int col;
    int rowSpan;
    int colSpan;

    GridCell(int r = 0, int c = 0, int rs = 1, int cs = 1)
        : row(r)
        , col(c)
        , rowSpan(rs)
        , colSpan(cs)
    {
    }

    bool operator==(const GridCell& other) const
    {
        return row == other.row && col == other.col && rowSpan == other.rowSpan
            && colSpan == other.colSpan;
    }
};

struct GridArea {
    int startRow;
    int startCol;
    int endRow;
    int endCol;

    GridArea(int sr = 0, int sc = 0, int er = 0, int ec = 0)
        : startRow(sr)
        , startCol(sc)
        , endRow(er)
        , endCol(ec)
    {
    }

    bool isValid() const
    {
        return startRow <= endRow && startCol <= endCol;
    }

    GridCell toCell() const
    {
        return GridCell(
            startRow, startCol, endRow - startRow + 1, endCol - startCol + 1);
    }
};

enum class LayoutType {
    IDE_STYLE,
    DOCUMENT_COMPARE,
    TRIPLE_COLUMNS,
    QUAD_GRID,
    MASTER_DETAIL,
    PRESENTATION,
    CUSTOM,
};

struct WindowConstraints {
    int minWidth = 100;
    int minHeight = 100;
    int maxWidth = 16777215;
    int maxHeight = 16777215;
    bool keepAspectRatio = false;
    bool allowResize = true;
    bool allowDrag = true;
};

struct AnimationConfig {
    int duration = 300;
    QEasingCurve::Type easing = QEasingCurve::OutCubic;
    bool enabled = true;
};

}

#endif // TYPES_H
