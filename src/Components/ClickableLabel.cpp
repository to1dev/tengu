#include "ClickableLabel.h"

namespace Daitengu::Components {

ClickableLabel::ClickableLabel(QWidget* parent)
    : QLabel(parent)
{
    installEventFilter(this);
}

bool ClickableLabel::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == this && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            Q_EMIT clicked();
            return true;
        }
    }
    return QLabel::eventFilter(watched, event);
}

}
