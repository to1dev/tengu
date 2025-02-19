#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QMouseEvent>

namespace Daitengu::Components {

class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(QWidget* parent = nullptr);

Q_SIGNALS:
    void clicked();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
};

}
#endif // CLICKABLELABEL_H
