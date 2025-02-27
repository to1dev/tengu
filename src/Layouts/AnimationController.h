#ifndef ANIMATIONCONTROLLER_H
#define ANIMATIONCONTROLLER_H

#include <QMap>
#include <QObject>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QRect>
#include <QWidget>

namespace Daitengu::Layouts {

class AnimationController : public QObject {
    Q_OBJECT

public:
    explicit AnimationController(QObject* parent = nullptr);
    ~AnimationController() override;

    void animateTo(
        const QMap<QWidget*, QRect>& targetGeometries, int duration = 300);

Q_SIGNALS:
    void animationFinished();

private Q_SLOTS:
    void onGroupFinished();

private:
    QParallelAnimationGroup* group_;
};

}
#endif // ANIMATIONCONTROLLER_H
