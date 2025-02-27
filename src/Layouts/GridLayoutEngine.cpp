#include "GridLayoutEngine.h"

namespace Daitengu::Layouts {

GridLayoutEngine::GridLayoutEngine(
    int rows, int cols, int hGap, int vGap, const QMargins& padding)
    : rows_(rows)
    , cols_(cols)
    , hGap_(hGap)
    , vGap_(vGap)
    , padding_(padding)
{
}

QMap<QWidget*, QRect> GridLayoutEngine::calculateLayout(
    const QList<WindowState*>& windows, const QRect& screenGeometry)
{
    QMap<QWidget*, QRect> result;

    if (rows_ <= 0 || cols_ <= 0 || screenGeometry.isEmpty()
        || windows.isEmpty()) {
        for (auto* wstate : windows) {
            if (auto* w = wstate->widget()) {
                result[w] = QRect(screenGeometry.topLeft(), w->size());
            }
        }
        return result;
    }

    QRect adjustedScreen = screenGeometry.adjusted(
        padding_.left(), padding_.top(), -padding_.right(), -padding_.bottom());

    double cellWidth = (adjustedScreen.width() - (cols_ - 1) * hGap_)
        / static_cast<double>(cols_);
    double cellHeight = (adjustedScreen.height() - (rows_ - 1) * vGap_)
        / static_cast<double>(rows_);

    int index = 0;
    for (auto* wstate : windows) {
        int row = index / cols_;
        int col = index % cols_;
        if (row >= rows_) {
            break;
        }

        int x = adjustedScreen.x() + col * (cellWidth + hGap_);
        int y = adjustedScreen.y() + row * (cellHeight + vGap_);
        int w = static_cast<int>(cellWidth);
        int h = static_cast<int>(cellHeight);

        QRect target(x, y, w, h);

        const auto& c = wstate->constraints();
        QSize constrainedSize = target.size();
        constrainedSize.setWidth(
            std::clamp(constrainedSize.width(), c.minWidth, c.maxWidth));
        constrainedSize.setHeight(
            std::clamp(constrainedSize.height(), c.minHeight, c.maxHeight));
        target.setSize(constrainedSize);

        if (c.keepAspectRatio) {
            double desiredAspect = static_cast<double>(wstate->initialWidth())
                / static_cast<double>(wstate->initialHeight());
            if (desiredAspect > 0.0) {
                double actualAspect
                    = static_cast<double>(target.width()) / target.height();
                if (actualAspect > desiredAspect) {
                    int newWidth
                        = static_cast<int>(target.height() * desiredAspect);
                    target.setX(target.x() + (target.width() - newWidth) / 2);
                    target.setWidth(newWidth);
                } else {
                    int newHeight
                        = static_cast<int>(target.width() / desiredAspect);
                    target.setY(target.y() + (target.height() - newHeight) / 2);
                    target.setHeight(newHeight);
                }
            }
        }

        if (auto* wdg = wstate->widget()) {
            result[wdg] = target;
        }
        ++index;
    }

    return result;
}

}
