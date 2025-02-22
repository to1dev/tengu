#ifndef MNEMONICVIEW_H
#define MNEMONICVIEW_H

#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QGraphicsView>
#include <QLabel>

#include "FlowLayout.h"

namespace Daitengu::Components {

inline constexpr char VIEW_MNEMONIC[] = "viewMnemonic";
inline constexpr char LABEL_MNEMONIC[] = "labelMnemonic";

class MnemonicView : public QGraphicsView {
    Q_OBJECT

    static inline constexpr int VIEW_MARGIN = 5;

public:
    explicit MnemonicView(QWidget* parent = nullptr);
    void clear();
    void myFitInView();

    QString mnemonic() const;
    void setMnemonic(const QString& newMnemonic);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QGraphicsScene scene_;
    QGraphicsWidget* widget_ { nullptr };

    QString mnemonic_;
};

}
#endif // MNEMONICVIEW_H
