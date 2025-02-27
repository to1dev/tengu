#include "AnimationController.h"

namespace Daitengu::Layouts {

AnimationController::AnimationController(QObject* parent)
    : QObject(parent)
    , group_(nullptr)
{
}

AnimationController::~AnimationController()
{
    if (group_) {
        group_->stop();
        delete group_;
    }
}

void AnimationController::animateTo(
    const QMap<QWidget*, QRect>& targetGeometries, int duration)
{
    if (group_) {
        group_->stop();
        delete group_;
        group_ = nullptr;
    }

    group_ = new QParallelAnimationGroup(this);

    for (auto it = targetGeometries.begin(); it != targetGeometries.end();
        ++it) {
        QWidget* w = it.key();
        if (!w)
            continue;
        QRect target = it.value();

        QPropertyAnimation* anim
            = new QPropertyAnimation(w, "geometry", group_);
        anim->setDuration(duration);
        anim->setStartValue(w->geometry());
        anim->setEndValue(target);
        // or easingCurve
        // anim->setEasingCurve(QEasingCurve::OutCubic);

        group_->addAnimation(anim);
    }

    connect(group_, &QParallelAnimationGroup::finished, this,
        &AnimationController::onGroupFinished);

    group_->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationController::onGroupFinished()
{
    Q_EMIT animationFinished();
    group_ = nullptr;
}

}
