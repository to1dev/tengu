#include "SVGWidget.h"

namespace Daitengu::Components {

SVGWidget::SVGWidget(const QString& file, QWidget* parent)
    : QWidget(parent)
{
    renderer_ = new QSvgRenderer(file, this);
}

void SVGWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    renderer_->render(&painter, rect());
}

}
