#include "LayoutFactory.h"

namespace Daitengu::Layouts {

LayoutFactory::LayoutGenerator LayoutFactory::getGenerator(LayoutType type)
{
    auto& registry = getRegistry();
    auto it = registry.find(type);
    if (it != registry.end()) {
        return it->generator;
    }
    return nullptr;
}

LayoutFactory::LayoutGenerator LayoutFactory::createCustomGenerator(
    const QVector<GridArea>& areas)
{
    return nullptr;
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
    auto it = registry.find(type);
    return it != registry.end() ? it->name : QString();
}

QString LayoutFactory::getLayoutDescription(LayoutType type)
{
    auto& registry = getRegistry();
    auto it = registry.find(type);
    return it != registry.end() ? it->description : QString();
}

QMap<QWidget*, GridArea> LayoutFactory::generateIdeStyle(
    const QList<QWidget*>& windows)
{
    QMap<QWidget*, GridArea> layout;
    auto areas = PredefinedLayouts::getLayoutAreas(LayoutType::IDE_STYLE);

    for (int i = 0; i < qMin(windows.size(), areas.size()); ++i) {
        layout[windows[i]] = areas[i];
    }

    return layout;
}

QMap<QWidget*, GridArea> LayoutFactory::generateDocumentCompare(
    const QList<QWidget*>& windows)
{
    QMap<QWidget*, GridArea> layout;
    auto areas
        = PredefinedLayouts::getLayoutAreas(LayoutType::DOCUMENT_COMPARE);

    for (int i = 0; i < qMin(windows.size(), areas.size()); ++i) {
        layout[windows[i]] = areas[i];
    }

    return layout;
}

QMap<QWidget*, GridArea> LayoutFactory::generateTripleColumns(
    const QList<QWidget*>& windows)
{
    QMap<QWidget*, GridArea> layout;
    auto areas = PredefinedLayouts::getLayoutAreas(LayoutType::TRIPLE_COLUMNS);

    for (int i = 0; i < qMin(windows.size(), areas.size()); ++i) {
        layout[windows[i]] = areas[i];
    }

    return layout;
}

QMap<QWidget*, GridArea> LayoutFactory::generateQuadGrid(
    const QList<QWidget*>& windows)
{
    QMap<QWidget*, GridArea> layout;
    auto areas = PredefinedLayouts::getLayoutAreas(LayoutType::QUAD_GRID);

    for (int i = 0; i < qMin(windows.size(), areas.size()); ++i) {
        layout[windows[i]] = areas[i];
    }

    return layout;
}

QMap<QWidget*, GridArea> LayoutFactory::generateMasterDetail(
    const QList<QWidget*>& windows)
{
    QMap<QWidget*, GridArea> layout;
    auto areas = PredefinedLayouts::getLayoutAreas(LayoutType::MASTER_DETAIL);

    for (int i = 0; i < qMin(windows.size(), areas.size()); ++i) {
        layout[windows[i]] = areas[i];
    }

    return layout;
}

QMap<QWidget*, GridArea> LayoutFactory::generatePresentation(
    const QList<QWidget*>& windows)
{
    QMap<QWidget*, GridArea> layout;
    auto areas = PredefinedLayouts::getLayoutAreas(LayoutType::PRESENTATION);

    for (int i = 0; i < qMin(windows.size(), areas.size()); ++i) {
        layout[windows[i]] = areas[i];
    }

    return layout;
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
    }

    return registry;
}

}
