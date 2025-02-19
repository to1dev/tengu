#include "SVGWidget.h"

namespace Daitengu::Components {

SVGWidget::SVGWidget(const QString& file, QWidget* parent)
    : QWidget(parent)
{
    renderer_ = new QSvgRenderer(file, this);
    installEventFilter(this);
}

void SVGWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    renderer_->render(&painter, rect());
}

bool SVGWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == this && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            Q_EMIT clicked();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

}
