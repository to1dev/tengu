#include "LineEditEx.h"

namespace Daitengu::Components {

LineEditEx::LineEditEx(QWidget* parent)
    : QLineEdit(parent)
{
    setClearButtonEnabled(true);

    QToolButton* clearButton = findChild<QToolButton*>();
    if (clearButton) {
        clearButton->setIcon(QIcon(":/Media/Xmark"));
    }
}

}
