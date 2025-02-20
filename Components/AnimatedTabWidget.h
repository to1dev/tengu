#ifndef ANIMATEDTABWIDGET_H
#define ANIMATEDTABWIDGET_H

#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QRect>
#include <QTabBar>
#include <QTabWidget>

namespace Daitengu::Components {

class AnimatedTabWidget : public QTabWidget {
    Q_OBJECT

public:
    enum class AnimationType {
        Fade,
        SlideHorizontal,
        SlideVertical,
        SlideFade,
    };

    explicit AnimatedTabWidget(QWidget* parent = nullptr);

    void setAnimationDuration(int newAnimationDuration);

    void setAnimationType(AnimationType newAnimationType);

    void setCurrentIndexEx(int index);

private Q_SLOTS:
    void onTabBarClicked(int index);

private:
    void animateFade(int index);
    void animateSlide(int index, bool horizontal);
    void animateSlideFade(int index, bool horizontal);

    int animationDuration_;
    AnimationType animationType_;
};

}
#endif // ANIMATEDTABWIDGET_H
