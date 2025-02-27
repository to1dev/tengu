#include "SVGWidget.h"

namespace Daitengu::Components {

SVGWidget::SVGWidget(
    const QString& file, QWidget* parent, int padding, bool clickable)
    : QWidget(parent)
    , padding_(padding)
    , clickable_(clickable)
{
    renderer_ = new QSvgRenderer(file, this);
    imageSize_ = renderer_->defaultSize();
    if (clickable)
        installEventFilter(this);
}

QSize SVGWidget::sizeHint() const
{
    return QSize(
        imageSize_.width() + 2 * padding_, imageSize_.height() + 2 * padding_);
}

void SVGWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QRect drawRect(padding_, padding_, imageSize_.width(), imageSize_.height());
    renderer_->render(&painter, drawRect);
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

QSize SVGWidget::imageSize() const
{
    return imageSize_;
}

void SVGWidget::setImageSize(const QSize& newImageSize)
{
    imageSize_ = newImageSize;
    setFixedSize(
        imageSize_.width() + 2 * padding_, imageSize_.height() + 2 * padding_);
    updateGeometry();
    update();
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
    setFixedSize(
        imageSize_.width() + 2 * padding_, imageSize_.height() + 2 * padding_);
    updateGeometry();
    update();
}

}
