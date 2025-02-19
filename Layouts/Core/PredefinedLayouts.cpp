#include "PredefinedLayouts.h"

namespace Daitengu::Layouts {

QMap<LayoutType, PredefinedLayouts::LayoutDescription>
PredefinedLayouts::getLayoutDescriptions()
{
    QMap<LayoutType, LayoutDescription> descriptions;

    descriptions[LayoutType::IDE_STYLE] = {
        "IDE Style",
        "Classic IDE layout with project explorer, main editor, and properties "
        "panel",
        getIdeStyleAreas(),
        1,
        3,
    };

    descriptions[LayoutType::DOCUMENT_COMPARE] = {
        "Document Compare",
        "Side by side document comparison view",
        getDocumentCompareAreas(),
        2,
        2,
    };

    descriptions[LayoutType::TRIPLE_COLUMNS] = {
        "Triple Columns",
        "Three equal width columns",
        getTripleColumnsAreas(),
        1,
        3,
    };

    descriptions[LayoutType::QUAD_GRID] = {
        "Quad Grid",
        "2x2 grid layout",
        getQuadGridAreas(),
        1,
        4,
    };

    descriptions[LayoutType::MASTER_DETAIL] = {
        "Master Detail",
        "Master view on left (1/3) with detail view on right (2/3)",
        getMasterDetailAreas(),
        2,
        2,
    };

    descriptions[LayoutType::PRESENTATION] = {
        "Presentation",
        "Main content with optional presenter notes below",
        getPresentationAreas(),
        1,
        2,
    };

    return descriptions;
}

QVector<GridArea> PredefinedLayouts::getLayoutAreas(LayoutType type)
{
    switch (type) {
    case LayoutType::IDE_STYLE:
        return getIdeStyleAreas();
    case LayoutType::DOCUMENT_COMPARE:
        return getDocumentCompareAreas();
    case LayoutType::TRIPLE_COLUMNS:
        return getTripleColumnsAreas();
    case LayoutType::QUAD_GRID:
        return getQuadGridAreas();
    case LayoutType::MASTER_DETAIL:
        return getMasterDetailAreas();
    case LayoutType::PRESENTATION:
        return getPresentationAreas();
    default:
        return QVector<GridArea>();
    }
}

QPair<int, int> PredefinedLayouts::getWindowCountRange(LayoutType type)
{
    const auto descriptions = getLayoutDescriptions();
    auto it = descriptions.find(type);
    if (it != descriptions.end()) {
        return qMakePair(it->minWindows, it->maxWindows);
    }
    return qMakePair(0, 0);
}

bool PredefinedLayouts::isLayoutApplicable(LayoutType type, int windowCount)
{
    auto range = getWindowCountRange(type);
    return windowCount >= range.first && windowCount <= range.second;
}

QMap<QWidget*, GridArea> PredefinedLayouts::getDefaultLayout(
    LayoutType type, const QList<QWidget*>& windows)
{
    QMap<QWidget*, GridArea> layout;
    auto areas = getLayoutAreas(type);

    for (int i = 0; i < qMin(windows.size(), areas.size()); ++i) {
        layout[windows[i]] = areas[i];
    }

    return layout;
}

QVector<GridArea> PredefinedLayouts::getIdeStyleAreas()
{
    return {
        GridArea(0, 0, 11, 2),
        GridArea(0, 3, 11, 8),
        GridArea(0, 9, 11, 11),
    };
}

QVector<GridArea> PredefinedLayouts::getDocumentCompareAreas()
{
    return {
        GridArea(0, 0, 11, 5),
        GridArea(0, 6, 11, 11),
    };
}

QVector<GridArea> PredefinedLayouts::getTripleColumnsAreas()
{
    return {
        GridArea(0, 0, 11, 3),
        GridArea(0, 4, 11, 7),
        GridArea(0, 8, 11, 11),
    };
}

QVector<GridArea> PredefinedLayouts::getQuadGridAreas()
{
    return {
        GridArea(0, 0, 5, 5),
        GridArea(0, 6, 5, 11),
        GridArea(6, 0, 11, 5),
        GridArea(6, 6, 11, 11),
    };
}

QVector<GridArea> PredefinedLayouts::getMasterDetailAreas()
{
    return {
        GridArea(0, 0, 11, 3),
        GridArea(0, 4, 11, 11),
    };
}

QVector<GridArea> PredefinedLayouts::getPresentationAreas()
{
    return {
        GridArea(0, 0, 8, 11),
        GridArea(9, 0, 11, 11),
    };
}

}
