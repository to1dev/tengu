#ifndef LAYOUTFACTORY_H
#define LAYOUTFACTORY_H

#include <functional>

#include "PredefinedLayouts.h"
#include "Types.h"

namespace Daitengu::Layouts {

class LayoutFactory {
public:
    using LayoutGenerator
        = std::function<QMap<QWidget*, GridArea>(const QList<QWidget*>&)>;

    static LayoutGenerator getGenerator(LayoutType type);

    static LayoutGenerator createCustomGenerator(
        const QVector<GridArea>& areas);

    static void registerLayout(LayoutType type,
        const LayoutGenerator& generator, const QString& name,
        const QString& description);

    static void unregisterLayout(LayoutType type);

    static bool isLayoutRegistered(LayoutType type);

    static QList<LayoutType> getRegisteredLayouts();

    static QString getLayoutName(LayoutType type);
    static QString getLayoutDescription(LayoutType type);

private:
    struct LayoutInfo {
        LayoutGenerator generator;
        QString name;
        QString description;
    };

    static QMap<LayoutType, LayoutInfo>& getRegistry();

    static QMap<QWidget*, GridArea> generateIdeStyle(
        const QList<QWidget*>& windows);
    static QMap<QWidget*, GridArea> generateDocumentCompare(
        const QList<QWidget*>& windows);
    static QMap<QWidget*, GridArea> generateTripleColumns(
        const QList<QWidget*>& windows);
    static QMap<QWidget*, GridArea> generateQuadGrid(
        const QList<QWidget*>& windows);
    static QMap<QWidget*, GridArea> generateMasterDetail(
        const QList<QWidget*>& windows);
    static QMap<QWidget*, GridArea> generatePresentation(
        const QList<QWidget*>& windows);
};

}
#endif // LAYOUTFACTORY_H
