#ifndef FRAMELESS_H
#define FRAMELESS_H

#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QObject>
#include <QToolButton>
#include <QWidget>

#include "Consts.h"

#include "Components/TitleBar.h"

#include "Utils/Encryption.h"

using namespace Daitengu::Base;
using namespace Daitengu::Components;
using namespace Daitengu::Utils;

namespace Daitengu::UI {

inline constexpr int MAIN_MARGIN = 20;
inline constexpr int CONTENT_MARGIN = 10;
inline constexpr int ICON_MARGIN = 3;

inline constexpr char STR_MAIN_NORMAL[] = "";
inline constexpr char STR_MAIN_MAX[] = "";
inline constexpr char STR_TITLE_BAR[] = "titleBar";
inline constexpr char STR_DRAG_BAR[] = "dragBar";
inline constexpr char STR_MAIN_ICON[] = ":/Logos/main.svg";
inline constexpr char STR_LABEL_TITLE[] = "labelTitle";

inline const QString STR_MAIN_TOOLTIP_MINIMIZE = QObject::tr("最小化应用程序");
inline const QString STR_MAIN_TOOLTIP_CLOSE = QObject::tr("退出应用程序");
inline const QString STR_FORM_TOOLTIP_CLOSE = QObject::tr("关闭窗体");
inline const QString STR_FORM_TOOLTIP_MAX = QObject::tr("最大化窗体");
inline const QString STR_FORM_TOOLTIP_NORMAL = QObject::tr("恢复窗体");
inline const QString STR_FORM_TOOLTIP_FIXED = QObject::tr("锁定窗体移动范围");

class Frameless : public QObject {
    Q_OBJECT
public:
    Frameless(QWidget* window = nullptr);
    virtual ~Frameless() = default;

    void init(const bool isMain = false);

    void setMainFrame(QWidget* newMainFrame);
    void setTopFrame(QWidget* newTopFrame);
    void setContentFrame(QWidget* newContentFrame);

    void setButtonMin(QToolButton* newButtonMin);
    void setButtonMax(QToolButton* newButtonMax);
    void setButtonClose(QToolButton* newButtonClose);
    void setButtonFixed(QToolButton* newButtonFixed);

    void setMainMenu(QMenuBar* newMainMenu);

    bool fixed() const;
    void setFixed(bool newFixed);

private:
    void min();
    void max();

private:
    QWidget* mWindow;
    QString mTitle;

    bool mFixed { false };

    QWidget* mMainFrame { nullptr };
    QWidget* mTopFrame { nullptr };
    QWidget* mContentFrame { nullptr };

    QToolButton* mButtonMin { nullptr };
    QToolButton* mButtonMax { nullptr };
    QToolButton* mButtonClose { nullptr };
    QToolButton* mButtonFixed { nullptr };

    QMenuBar* mMainMenu { nullptr };
};

}
#endif // FRAMELESS_H
