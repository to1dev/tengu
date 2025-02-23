#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include <QDebug>
#include <QGuiApplication>
#include <QList>
#include <QObject>
#include <QScreen>

namespace Daitengu::Layouts {

class ScreenManager : public QObject {
    Q_OBJECT

public:
    explicit ScreenManager(QObject* parent = nullptr);
    ~ScreenManager() override = default;

    QList<QScreen*> screens() const;

Q_SIGNALS:
    void screensChanged();

private Q_SLOTS:
    void handleScreenAdded(QScreen* screen);
    void handleScreenRemoved(QScreen* screen);

private:
    QList<QScreen*> screens_;
};

}
#endif // SCREENMANAGER_H
