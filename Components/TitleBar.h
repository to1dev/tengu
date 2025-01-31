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
    QWidget* mWindow { nullptr };
    bool mPressed { false };
    bool mFixed { false };
    QPoint mPos;
    QPoint mOldPos;
};

}
#endif // TITLEBAR_H
