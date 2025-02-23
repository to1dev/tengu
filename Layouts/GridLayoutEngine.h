#ifndef GRIDLAYOUTENGINE_H
#define GRIDLAYOUTENGINE_H

#include <QMargins>

#include "LayoutEngine.h"
#include "WindowState.h"

namespace Daitengu::Layouts {

class GridLayoutEngine : public LayoutEngine {
public:
    explicit GridLayoutEngine(int rows = 12, int cols = 12, int hGap = 0,
        int vGap = 0, const QMargins& padding = QMargins(0, 0, 0, 0));
    ~GridLayoutEngine() override = default;

    QMap<QWidget*, QRect> calculateLayout(const QList<WindowState*>& windows,
        const QRect& screenGeometry) override;

private:
    int rows_;
    int cols_;
    int hGap_;
    int vGap_;
    QMargins padding_;
};

}
#endif // GRIDLAYOUTENGINE_H
