#ifndef GRIDSYSTEM_H
#define GRIDSYSTEM_H

#include <QScreen>
#include <QVector>

#include "Types.h"

namespace Daitengu::Layouts {

class GridSystem {
public:
    GridSystem(int rows = 12, int cols = 12);

private:
    int rows_;
    int cols_;
    QRect screenGeometry_;
    int horizontalGap_;
    int verticalGap_;
    QVector<QVector<bool>> occupied_;
};

}
#endif // GRIDSYSTEM_H
