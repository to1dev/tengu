#ifndef WINDOWSTATE_H
#define WINDOWSTATE_H

#include <QWidget>

namespace Daitengu::Layouts {

struct WindowConstraints {
    int minWidth { 50 };
    int minHeight { 50 };
    int maxWidth { 10000 };
    int maxHeight { 10000 };
    bool keepAspectRatio { false };
};

class WindowState {
public:
    explicit WindowState(QWidget* w);

    QWidget* widget() const;

    WindowConstraints constraints() const;
    void setConstraints(const WindowConstraints newConstraints);

    void setInitialSize(int width, int height);
    int initialWidth() const;
    int initialHeight() const;

private:
    QWidget* widget_ { nullptr };
    WindowConstraints constraints_;
    int initialWidth_ { 800 };
    int initialHeight_ { 600 };
};

}
#endif // WINDOWSTATE_H
