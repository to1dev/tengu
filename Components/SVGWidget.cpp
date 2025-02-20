#include "SVGWidget.h"

namespace Daitengu::Components {

SVGWidget::SVGWidget(
    const QString& file, QWidget* parent, int padding, bool clickable)
    : QWidget(parent)
    , padding_(padding)
    , clickable_(clickable)
{
    renderer_ = new QSvgRenderer(file, this);
    if (clickable)
        installEventFilter(this);
}

void SVGWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QRect paddedRect
        = rect().adjusted(padding_, padding_, -padding_, -padding_);

    renderer_->render(&painter, paddedRect);
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

bool SVGWidget::clickable() const
{
    return clickable_;
}

void SVGWidget::setClickable(bool newClickable)
{
    clickable_ = newClickable;
    // TODO: re-install eventfilter
}

int SVGWidget::padding() const
{
    return padding_;
}

void SVGWidget::setPadding(int newPadding)
{
    padding_ = newPadding;
}

}
