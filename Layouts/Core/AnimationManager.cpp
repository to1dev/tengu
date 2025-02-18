#include "AnimationManager.h"

namespace Daitengu::Layouts {

AnimationManager::AnimationManager(QObject* parent)
    : QObject(parent)
{
    defaultConfig_.duration = 300;
    defaultConfig_.easing = QEasingCurve::OutCubic;
    defaultConfig_.enabled = true;
}

AnimationManager::~AnimationManager()
{
    stopAll();
}

void AnimationManager::startAnimation(
    QWidget* window, const QRect& targetGeometry, const AnimationConfig& config)
{
    if (!window || !config.enabled) {
        if (window) {
            window->setGeometry(targetGeometry);
        }
        return;
    }

    cleanupAnimation(window);

    auto* animation = createAnimation(window, targetGeometry, config);
    activeAnimations_[window] = animation;

    connect(animation, &QPropertyAnimation::finished, this, [this, window]() {
        cleanupAnimation(window);
        Q_EMIT animationFinished(window);
    });

    Q_EMIT animationStarted(window);
    animation->start();
}

void AnimationManager::startGroupAnimation(
    const QMap<QWidget*, QRect>& animations, const AnimationConfig& config)
{
    if (!config.enabled) {
        for (auto it = animations.begin(); it != animations.end(); ++it) {
            if (it.key()) {
                it.key()->setGeometry(it.value());
            }
        }
        return;
    }

    auto* group = new QParallelAnimationGroup(this);

    for (auto it = animations.begin(); it != animations.end(); ++it) {
        QWidget* window = it.key();
        if (!window)
            continue;

        cleanupAnimation(window);
        auto* animation = createAnimation(window, it.value(), config);
        group->addAnimation(animation);
    }

    connect(group, &QParallelAnimationGroup::finished, this, [this, group]() {
        cleanupGroup(group);
        Q_EMIT groupAnimationFinished();
    });

    activeGroups_.append(group);
    Q_EMIT groupAnimationStarted();
    group->start();
}

void AnimationManager::stopAll()
{
    for (auto it = activeAnimations_.begin(); it != activeAnimations_.end();
        ++it) {
        it.value()->stop();
        delete it.value();
    }
    activeAnimations_.clear();

    for (auto* group : activeGroups_) {
        group->stop();
        delete group;
    }
    activeGroups_.clear();
}

bool AnimationManager::isRunning() const
{
    return !activeAnimations_.isEmpty() || !activeGroups_.isEmpty();
}

void AnimationManager::setDefaultConfig(const AnimationConfig& config)
{
    defaultConfig_ = config;
}

AnimationConfig AnimationManager::defaultConfig() const
{
    return defaultConfig_;
}

void AnimationManager::cleanupAnimation(QWidget* window)
{
    auto it = activeAnimations_.find(window);
    if (it != activeAnimations_.end()) {
        it.value()->stop();
        delete it.value();
        activeAnimations_.remove(window);
    }
}

void AnimationManager::cleanupGroup(QParallelAnimationGroup* group)
{
    activeGroups_.removeOne(group);
    group->deleteLater();
}

QPropertyAnimation* AnimationManager::createAnimation(
    QWidget* window, const QRect& targetGeometry, const AnimationConfig& config)
{
    auto* animation = new QPropertyAnimation(window, "geometry", this);
    animation->setStartValue(window->geometry());
    animation->setEndValue(targetGeometry);
    animation->setDuration(config.duration);
    animation->setEasingCurve(config.easing);
    return animation;
}

}
