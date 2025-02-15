#ifndef SVGWIDGET_H
#define SVGWIDGET_H

#include <QPainter>
#include <QSvgRenderer>
#include <QWidget>

namespace Daitengu::Components {

class SVGWidget : public QWidget {
public:
    SVGWidget(const QString& file, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QSvgRenderer* renderer_;
};

}
#endif // SVGWIDGET_H
