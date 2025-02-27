#include "PredefinedLayoutEngine.h"

namespace Daitengu::Layouts {

std::unique_ptr<LayoutEngine> PredefinedLayoutEngine::create(
    PredefinedType type)
{
    switch (type) {
    case PredefinedType::IDE_STYLE:
        return std::make_unique<GridLayoutEngine>(12, 12, 3, 3);

    case PredefinedType::DOCUMENT_COMPARE:
        return std::make_unique<GridLayoutEngine>(1, 2, 5, 5);

    case PredefinedType::TRIPLE_COLUMNS:
        return std::make_unique<GridLayoutEngine>(1, 3, 5, 5);

    case PredefinedType::QUAD_GRID:
        return std::make_unique<GridLayoutEngine>(2, 2, 5, 5);

    case PredefinedType::MASTER_DETAIL:
        return std::make_unique<GridLayoutEngine>(1, 2, 10, 0);

    case PredefinedType::PRESENTATION:
        return std::make_unique<GridLayoutEngine>(2, 1, 5, 5);

    default:
        return std::make_unique<GridLayoutEngine>(12, 12, 3, 3);
    }
}

}
