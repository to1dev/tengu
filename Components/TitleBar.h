#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QApplication>
#include <QMouseEvent>
#include <QScreen>
#include <QWidget>

namespace Daitengu::Components {

class TitleBar : public QWidget {
    Q_OBJECT

public:
    TitleBar(QWidget* parent = nullptr);

    bool fixed() const;
    void setFixed(bool newFixed);

signals:
    void doubleClick();

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    QWidget* window_ { nullptr };
    bool pressed_ { false };
    bool fixed_ { false };
    QPoint pos_;
    QPoint oldPos_;
};

}
#endif // TITLEBAR_H
