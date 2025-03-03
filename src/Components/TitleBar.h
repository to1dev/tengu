#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QApplication>
#include <QMouseEvent>
#include <QScreen>
#include <QWidget>

namespace Daitengu::Components {

static constexpr int SNAP_DISTANCE = 15;

class TitleBar : public QWidget {
    Q_OBJECT

public:
    TitleBar(QWidget* parent = nullptr);

    bool fixed() const;
    void setFixed(bool newFixed);

Q_SIGNALS:
    void doubleClick();

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    QPoint snapToScreenEdges(
        const QPoint& pos, const QRect& windowRect, const QRect& screenRect);

    QWidget* window_ { nullptr };
    bool pressed_ { false };
    bool fixed_ { false };
    QPoint pos_;
    QPoint oldPos_;
};

}
#endif // TITLEBAR_H
