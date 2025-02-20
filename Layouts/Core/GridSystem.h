#ifndef GRIDSYSTEM_H
#define GRIDSYSTEM_H

#include <algorithm>
#include <stdexcept>

#include <QScreen>
#include <QVector>

#include "Types.h"

namespace Daitengu::Layouts {

class GridSystem {
public:
    GridSystem(int rows = 12, int cols = 12);

    void setGridSize(int rows, int cols);
    void setScreenGeometry(const QRect& geometry);
    void setGap(int horizontal, int vertical);

    QRect cellToRect(const GridCell& cell) const;
    QRect areaToRect(const GridArea& area) const;
    GridCell rectToCell(const QRect& rect) const;
    GridArea rectToArea(const QRect& rect) const;

    bool isCellValid(const GridCell& cell) const;
    bool isAreaValid(const GridArea& area) const;
    bool isCellOccupied(const GridCell& cell) const;
    bool isAreaOccupied(const GridArea& area) const;
    void occupyCell(const GridCell& cell);
    void occupyArea(const GridArea& area);
    void releaseCell(const GridCell& cell);
    void releaseArea(const GridArea& area);
    void clear();

    GridArea getLeftHalf() const;
    GridArea getRightHalf() const;
    GridArea getTopHalf() const;
    GridArea getBottomHalf() const;
    GridArea getCenterArea() const;
    GridArea getFullArea() const;

    int rows() const;
    int cols();
    QRect screenGeometry() const;

    int margin() const;
    void setMargin(int newMargin);

private:
    int rows_;
    int cols_;
    QRect screenGeometry_;
    int horizontalGap_;
    int verticalGap_;
    int margin_;
    QVector<QVector<bool>> occupied_;

    void updateOccupancyGrid();
    QRect calculateCellRect(int row, int col, int rowSpan, int colSpan) const;
    void validateCell(const GridCell& cell) const;
    void validateArea(const GridArea& area) const;
};

}
#endif // GRIDSYSTEM_H
