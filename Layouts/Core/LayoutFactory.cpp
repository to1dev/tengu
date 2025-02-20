#include "LayoutFactory.h"

namespace Daitengu::Layouts {

LayoutFactory::LayoutGenerator LayoutFactory::getGenerator(LayoutType type)
{
    auto& registry = getRegistry();
    if (registry.contains(type))
        return registry[type].generator;
    return nullptr;
}

LayoutFactory::LayoutGenerator LayoutFactory::createCustomGenerator(
    const QVector<GridArea>& areas)
{
    return [areas](const QList<QWidget*>& windows) -> QMap<QWidget*, GridArea> {
        QMap<QWidget*, GridArea> layout;
        for (int i = 0; i < qMin(windows.size(), areas.size()); ++i)
            layout[windows[i]] = areas[i];
        return layout;
    };
}

void LayoutFactory::registerLayout(LayoutType type,
    const LayoutGenerator& generator, const QString& name,
    const QString& description)
{
    auto& registry = getRegistry();
    registry[type] = { generator, name, description };
}

void LayoutFactory::unregisterLayout(LayoutType type)
{
    getRegistry().remove(type);
}

bool LayoutFactory::isLayoutRegistered(LayoutType type)
{
    return getRegistry().contains(type);
}

QList<LayoutType> LayoutFactory::getRegisteredLayouts()
{
    return getRegistry().keys();
}

QString LayoutFactory::getLayoutName(LayoutType type)
{
    auto& registry = getRegistry();
    return registry.contains(type) ? registry[type].name : QString();
}

QString LayoutFactory::getLayoutDescription(LayoutType type)
{
    auto& registry = getRegistry();
    return registry.contains(type) ? registry[type].description : QString();
}

QMap<QWidget*, GridArea> LayoutFactory::generateIdeStyle(
    const QList<QWidget*>& windows)
{
    return PredefinedLayouts::getDefaultLayout(LayoutType::IDE_STYLE, windows);
}

QMap<QWidget*, GridArea> LayoutFactory::generateDocumentCompare(
    const QList<QWidget*>& windows)
{
    return PredefinedLayouts::getDefaultLayout(
        LayoutType::DOCUMENT_COMPARE, windows);
}

QMap<QWidget*, GridArea> LayoutFactory::generateTripleColumns(
    const QList<QWidget*>& windows)
{
    return PredefinedLayouts::getDefaultLayout(
        LayoutType::TRIPLE_COLUMNS, windows);
}

QMap<QWidget*, GridArea> LayoutFactory::generateQuadGrid(
    const QList<QWidget*>& windows)
{
    return PredefinedLayouts::getDefaultLayout(LayoutType::QUAD_GRID, windows);
}

QMap<QWidget*, GridArea> LayoutFactory::generateMasterDetail(
    const QList<QWidget*>& windows)
{
    return PredefinedLayouts::getDefaultLayout(
        LayoutType::MASTER_DETAIL, windows);
}

QMap<QWidget*, GridArea> LayoutFactory::generatePresentation(
    const QList<QWidget*>& windows)
{
    return PredefinedLayouts::getDefaultLayout(
        LayoutType::PRESENTATION, windows);
}

QMap<LayoutType, LayoutFactory::LayoutInfo>& LayoutFactory::getRegistry()
{
    static QMap<LayoutType, LayoutInfo> registry;

    if (registry.isEmpty()) {
        registry[LayoutType::IDE_STYLE] = {
            &generateIdeStyle,
            "IDE Style",
            "Left sidebar, main content, and right sidebar layout",
        };
        registry[LayoutType::DOCUMENT_COMPARE] = {
            &generateDocumentCompare,
            "Document Compare",
            "Side by side document comparison view",
        };
        registry[LayoutType::TRIPLE_COLUMNS] = {
            &generateTripleColumns,
            "Triple Columns",
            "Three equal width columns",
        };
        registry[LayoutType::QUAD_GRID] = {
            &generateQuadGrid,
            "Quad Grid",
            "2x2 grid layout",
        };
        registry[LayoutType::MASTER_DETAIL] = {
            &generateMasterDetail,
            "Master Detail",
            "Master view and detail view layout",
        };
        registry[LayoutType::PRESENTATION] = {
            &generatePresentation,
            "Presentation",
            "Main content with presenter notes",
        };
    }

    return registry;
}

}
