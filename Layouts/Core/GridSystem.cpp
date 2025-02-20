#include "GridSystem.h"

namespace Daitengu::Layouts {

GridSystem::GridSystem(int rows, int cols)
    : rows_(rows)
    , cols_(cols)
    , horizontalGap_(0)
    , verticalGap_(0)
    , margin_(0)
{
    updateOccupancyGrid();
}

void GridSystem::setGridSize(int rows, int cols)
{
    if (rows_ <= 0 || cols_ <= 0) {
        throw std::invalid_argument("Grid dimensions must be positive.");
    }

    rows_ = rows;
    cols_ = cols;
    updateOccupancyGrid();
}

void GridSystem::setScreenGeometry(const QRect& geometry)
{
    screenGeometry_ = geometry;
}

void GridSystem::setGap(int horizontal, int vertical)
{
    horizontalGap_ = horizontal;
    verticalGap_ = vertical;
}

QRect GridSystem::cellToRect(const GridCell& cell) const
{
    validateCell(cell);
    return calculateCellRect(cell.row, cell.col, cell.rowSpan, cell.colSpan);
}

QRect GridSystem::areaToRect(const GridArea& area) const
{
    validateArea(area);
    return calculateCellRect(area.startRow, area.startCol,
        area.endRow - area.startRow + 1, area.endCol - area.startCol + 1);
}

GridCell GridSystem::rectToCell(const QRect& rect) const
{
    double cellWidth = screenGeometry_.width() / static_cast<double>(cols_);
    double cellHeight = screenGeometry_.height() / static_cast<double>(rows_);

    int col = static_cast<int>((rect.x() - screenGeometry_.x()) / cellWidth);
    int row = static_cast<int>((rect.y() - screenGeometry_.y()) / cellHeight);
    int colSpan = static_cast<int>(rect.width() / cellWidth + 0.5);
    int rowSpan = static_cast<int>(rect.height() / cellHeight + 0.5);

    return GridCell(row, col, rowSpan, colSpan);
}

GridArea GridSystem::rectToArea(const QRect& rect) const
{
    double cellWidth = screenGeometry_.width() / static_cast<double>(cols_);
    double cellHeight = screenGeometry_.height() / static_cast<double>(rows_);

    int startCol
        = static_cast<int>((rect.left() - screenGeometry_.left()) / cellWidth);
    int startRow
        = static_cast<int>((rect.top() - screenGeometry_.top()) / cellHeight);
    int endCol
        = static_cast<int>((rect.right() - screenGeometry_.left()) / cellWidth);
    int endRow = static_cast<int>(
        (rect.bottom() - screenGeometry_.top()) / cellHeight);

    return GridArea(std::clamp(startRow, 0, rows_ - 1),
        std::clamp(startCol, 0, cols_ - 1), std::clamp(endRow, 0, rows_ - 1),
        std::clamp(endCol, 0, cols_ - 1));
}

bool GridSystem::isCellValid(const GridCell& cell) const
{
    return (cell.row >= 0) && (cell.row < rows_) && (cell.col >= 0)
        && (cell.col < cols_) && (cell.rowSpan > 0)
        && (cell.row + cell.rowSpan <= rows_) && (cell.colSpan > 0)
        && (cell.col + cell.colSpan <= cols_);
}

bool GridSystem::isAreaValid(const GridArea& area) const
{
    return (area.startRow >= 0) && (area.startRow < rows_)
        && (area.startCol >= 0) && (area.startCol < cols_)
        && (area.endRow >= area.startRow) && (area.endRow < rows_)
        && (area.endCol >= area.startCol) && (area.endCol < cols_);
}

bool GridSystem::isCellOccupied(const GridCell& cell) const
{
    if (!isCellValid(cell))
        return false;

    for (int r = cell.row; r < cell.row + cell.rowSpan; ++r) {
        for (int c = cell.col; c < cell.col + cell.colSpan; ++c) {
            if (occupied_[r][c])
                return true;
        }
    }

    return false;
}

bool GridSystem::isAreaOccupied(const GridArea& area) const
{
    validateArea(area);
    for (int r = area.startRow; r <= area.endRow; ++r) {
        for (int c = area.startCol; c <= area.endCol; ++c) {
            if (occupied_[r][c])
                return true;
        }
    }
    return false;
}

void GridSystem::occupyCell(const GridCell& cell)
{
    if (!isCellValid(cell))
        return;
    occupied_[cell.row][cell.col] = true;
}

void GridSystem::occupyArea(const GridArea& area)
{
    validateArea(area);
    for (int r = area.startRow; r <= area.endRow; ++r) {
        for (int c = area.startCol; c <= area.endCol; ++c) {
            occupied_[r][c] = true;
        }
    }
}

void GridSystem::releaseCell(const GridCell& cell)
{
    if (!isCellValid(cell))
        return;
    occupied_[cell.row][cell.col] = false;
}

void GridSystem::releaseArea(const GridArea& area)
{
    validateArea(area);
    for (int r = area.startRow; r <= area.endRow; ++r) {
        for (int c = area.startCol; c <= area.endCol; ++c) {
            occupied_[r][c] = false;
        }
    }
}

void GridSystem::clear()
{
    for (auto& row : occupied_) {
        row.fill(false);
    }
}

GridArea GridSystem::getLeftHalf() const
{
    return GridArea(0, 0, rows_ - 1, (cols_ / 2) - 1);
}

GridArea GridSystem::getRightHalf() const
{
    return GridArea(0, cols_ / 2, rows_ - 1, cols_ - 1);
}

GridArea GridSystem::getTopHalf() const
{
    return GridArea(0, 0, (rows_ / 2) - 1, cols_ - 1);
}

GridArea GridSystem::getBottomHalf() const
{
    return GridArea(rows_ / 2, 0, rows_ - 1, cols_ - 1);
}

GridArea GridSystem::getCenterArea() const
{
    int rowStart = rows_ / 4;
    int rowEnd = (rows_ * 3) / 4;
    int colStart = cols_ / 4;
    int colEnd = (cols_ * 3) / 4;
    return GridArea(rowStart, colStart, rowEnd, colEnd);
}

GridArea GridSystem::getFullArea() const
{
    return GridArea(0, 0, rows_ - 1, cols_ - 1);
}

int GridSystem::rows() const
{
    return rows_;
}

int GridSystem::cols()
{
    return cols_;
}

QRect GridSystem::screenGeometry() const
{
    return screenGeometry_;
}

int GridSystem::margin() const
{
    return margin_;
}

void GridSystem::setMargin(int newMargin)
{
    margin_ = newMargin;
}

void GridSystem::updateOccupancyGrid()
{
    occupied_.resize(rows_);
    for (auto& row : occupied_) {
        row.resize(cols_);
        row.fill(false);
    }
}

QRect GridSystem::calculateCellRect(
    int row, int col, int rowSpan, int colSpan) const
{
    double cellWidth = (screenGeometry_.width() - (cols_ - 1) * horizontalGap_)
        / static_cast<double>(cols_);
    double cellHeight = (screenGeometry_.height() - (rows_ - 1) * verticalGap_)
        / static_cast<double>(rows_);

    int x = screenGeometry_.x() + col * (cellWidth + horizontalGap_);
    int y = screenGeometry_.y() + row * (cellHeight + verticalGap_);
    int width = static_cast<int>(
        colSpan * cellWidth + (colSpan - 1) * horizontalGap_);
    int height
        = static_cast<int>(rowSpan * cellHeight + (rowSpan - 1) * verticalGap_);

    return QRect(x, y, width, height);
}

void GridSystem::validateCell(const GridCell& cell) const
{
    if (cell.row < 0 || cell.row >= rows_ || cell.col < 0 || cell.col >= cols_
        || cell.rowSpan <= 0 || cell.colSpan <= 0
        || cell.row + cell.rowSpan > rows_ || cell.col + cell.colSpan > cols_) {
        throw std::out_of_range("Invalid grid cell coordinates.");
    }
}

void GridSystem::validateArea(const GridArea& area) const
{
    if (!area.isValid() || area.startRow < 0 || area.startRow >= rows_
        || area.startCol < 0 || area.startCol >= cols_ || area.endRow < 0
        || area.endRow >= rows_ || area.endCol < 0 || area.endCol >= cols_) {
        throw std::out_of_range("Invalid grid area coordinates.");
    }
}

}
