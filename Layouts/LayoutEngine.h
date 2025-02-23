#ifndef LAYOUTENGINE_H
#define LAYOUTENGINE_H

#include <QList>
#include <QMap>
#include <QRect>
#include <QWidget>

namespace Daitengu::Layouts {

class WindowState;

class LayoutEngine {
public:
    virtual ~LayoutEngine() = default;

    virtual QMap<QWidget*, QRect> calculateLayout(
        const QList<WindowState*>& windows, const QRect& screenGeometry)
        = 0;
};

}
#endif // LAYOUTENGINE_H
