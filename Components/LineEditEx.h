#ifndef LINEEDITEX_H
#define LINEEDITEX_H

#include <QLineEdit>
#include <QToolButton>

namespace Daitengu::Components {

class LineEditEx final : public QLineEdit {
    Q_OBJECT

public:
    explicit LineEditEx(QWidget* parent = nullptr);
};

}
#endif // LINEEDITEX_H
