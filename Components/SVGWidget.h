#ifndef SVGWIDGET_H
#define SVGWIDGET_H

#include <QMouseEvent>
#include <QPainter>
#include <QSvgRenderer>
#include <QWidget>

namespace Daitengu::Components {

class SVGWidget : public QWidget {
    Q_OBJECT

public:
    SVGWidget(const QString& file, QWidget* parent = nullptr, int padding = 0,
        bool clickable = false);

    int padding() const;
    void setPadding(int newPadding);

    bool clickable() const;
    void setClickable(bool newClickable);

    QSize imageSize() const;
    void setImageSize(const QSize &newImageSize);

Q_SIGNALS:
    void clicked();

protected:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QSvgRenderer* renderer_;
    int padding_ { 0 };
    bool clickable_ { false };
    QSize imageSize_;
};

}
#endif // SVGWIDGET_H
