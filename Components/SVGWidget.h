#ifndef SVGWIDGET_H
#define SVGWIDGET_H

#include <QMouseEvent>
#include <QPainter>
#include <QSvgRenderer>
#include <QWidget>

namespace Daitengu::Components {

class SVGWidget : public QWidget {
    Q_OBJECT

public:
    SVGWidget(const QString& file, QWidget* parent = nullptr);

Q_SIGNALS:
    void clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QSvgRenderer* renderer_;
};

}
#endif // SVGWIDGET_H
