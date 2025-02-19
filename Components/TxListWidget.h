#ifndef TXLISTWIDGET_H
#define TXLISTWIDGET_H

#include <QListWidget>
#include <QScrollBar>

namespace Daitengu::Components {

class TxListWidget : public QListWidget {
public:
    TxListWidget(QWidget* parent = nullptr);
    ~TxListWidget();
};

}
#endif // TXLISTWIDGET_H
