#ifndef PREDEFINEDLAYOUTENGINE_H
#define PREDEFINEDLAYOUTENGINE_H

#include <memory>

#include "GridLayoutEngine.h"
#include "LayoutEngine.h"

namespace Daitengu::Layouts {

enum class PredefinedType {
    IDE_STYLE,
    DOCUMENT_COMPARE,
    TRIPLE_COLUMNS,
    QUAD_GRID,
    MASTER_DETAIL,
    PRESENTATION
};

class PredefinedLayoutEngine {
public:
    static std::unique_ptr<LayoutEngine> create(PredefinedType type);
};

}
#endif // PREDEFINEDLAYOUTENGINE_H
