#ifndef LAYOUTORCHESTRATOR_H
#define LAYOUTORCHESTRATOR_H

#include <memory>

#include <QDebug>
#include <QGuiApplication>
#include <QMap>
#include <QObject>
#include <QRect>
#include <QScreen>

#include "AnimationController.h"
#include "LayoutEngine.h"
#include "ScreenManager.h"
#include "WindowState.h"

namespace Daitengu::Layouts {

class LayoutOrchestrator : public QObject {
    Q_OBJECT

public:
    explicit LayoutOrchestrator(QObject* parent = nullptr);
    ~LayoutOrchestrator() override;

    void setLayoutEngine(std::unique_ptr<LayoutEngine> newLayoutEngine);

    void registerWindow(QWidget* w);
    void unregisterWindow(QWidget* w);

    void snapToLayout();

    void animateToLayout(int duration = 300);

    ScreenManager* screenManager() const;

Q_SIGNALS:
    void layoutChanged();

private Q_SLOTS:
    void onScreensChanged();

private:
    std::unique_ptr<LayoutEngine> layoutEngine_;
    QList<WindowState*> windows_;
    AnimationController* animController_;
    ScreenManager* screenManager_;

    void applyLayout(const QMap<QWidget*, QRect>& layout);
};

}
#endif // LAYOUTORCHESTRATOR_H
