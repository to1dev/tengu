#ifndef ANIMATIONMANAGER_H
#define ANIMATIONMANAGER_H

#include <QMap>
#include <QObject>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

#include "Types.h"

namespace Daitengu::Layouts {

class AnimationManager : public QObject {
    Q_OBJECT

public:
    explicit AnimationManager(QObject* parent = nullptr);
    ~AnimationManager();

Q_SIGNALS:
    void animationStarted(QWidget* window);
};

}
#endif // ANIMATIONMANAGER_H
