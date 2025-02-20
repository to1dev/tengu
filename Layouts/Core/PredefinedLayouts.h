#ifndef PREDEFINEDLAYOUTS_H
#define PREDEFINEDLAYOUTS_H

#include <algorithm>

#include <QMap>
#include <QVector>
#include <QWidget>

#include "Types.h"

namespace Daitengu::Layouts {

class PredefinedLayouts {
public:
    struct LayoutDescription {
        QString name;
        QString description;
        QVector<GridArea> areas;
        int minWindows;
        int maxWindows;
    };

    static QMap<LayoutType, LayoutDescription> getLayoutDescriptions();
    static QVector<GridArea> getLayoutAreas(LayoutType type);
    static QPair<int, int> getWindowCountRange(LayoutType type);
    static bool isLayoutApplicable(LayoutType type, int windowCount);
    static QMap<QWidget*, GridArea> getDefaultLayout(
        LayoutType type, const QList<QWidget*>& windows);

private:
    static QVector<GridArea> getIdeStyleAreas();
    static QVector<GridArea> getDocumentCompareAreas();
    static QVector<GridArea> getTripleColumnsAreas();
    static QVector<GridArea> getQuadGridAreas();
    static QVector<GridArea> getMasterDetailAreas();
    static QVector<GridArea> getPresentationAreas();
};

}
#endif // PREDEFINEDLAYOUTS_H
