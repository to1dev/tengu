#ifndef ANIMATIONMANAGER_H
#define ANIMATIONMANAGER_H

#include <QMap>
#include <QObject>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QWidget>

#include "Types.h"

namespace Daitengu::Layouts {

class AnimationManager : public QObject {
    Q_OBJECT

public:
    explicit AnimationManager(QObject* parent = nullptr);
    ~AnimationManager();

    void startAnimation(QWidget* window, const QRect& targetGeometry,
        const AnimationConfig& config);
    void startGroupAnimation(
        const QMap<QWidget*, QRect>& animations, const AnimationConfig& config);
    void stopAll();
    bool isRunning() const;

    void setDefaultConfig(const AnimationConfig& config);
    AnimationConfig defaultConfig() const;

Q_SIGNALS:
    void animationStarted(QWidget* window);
    void animationFinished(QWidget* window);
    void groupAnimationStarted();
    void groupAnimationFinished();

private:
    QMap<QWidget*, QPropertyAnimation*> activeAnimations_;
    QList<QParallelAnimationGroup*> activeGroups_;
    AnimationConfig defaultConfig_;

    void cleanupAnimation(QWidget* window);
    void cleanupGroup(QParallelAnimationGroup* group);
    QPropertyAnimation* createAnimation(QWidget* window,
        const QRect& targetGeometry, const AnimationConfig& config);
};

}
#endif // ANIMATIONMANAGER_H
