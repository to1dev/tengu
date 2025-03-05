// SPDX-License-Identifier: AGPL-3.0-or-later
/*
 * Copyright (C) 2025 to1dev <https://arc20.me/to1dev>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef FRAMELESS_H
#define FRAMELESS_H

#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QObject>
#include <QToolButton>
#include <QWidget>

#include "Consts.h"

#include "Components/SVGWidget.h"
#include "Components/TitleBar.h"

#include "Utils/Helpers.hpp"

using namespace Daitengu::Core;
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

inline constexpr char STR_BUTTON_MIN[] = "ButtonMin";
inline constexpr char STR_BUTTON_MAX[] = "ButtonMax";
inline constexpr char STR_BUTTON_CLOSE[] = "ButtonClose";
inline constexpr char STR_BUTTON_FIXED[] = "ButtonFixed";

inline constexpr char STR_BUTTON_MIN_TEXT[] = "✔️";
inline constexpr char STR_BUTTON_MAX_TEXT[] = "🌀";
inline constexpr char STR_BUTTON_PINNED_TEXT[] = "📌";
inline constexpr char STR_BUTTON_FIXED_TEXT[] = "⚡️";
inline constexpr char STR_BUTTON_CLOSE_TEXT[] = "✖️";

inline const QString STR_MAIN_TOOLTIP_MINIMIZE = QObject::tr("最小化应用程序");
inline const QString STR_MAIN_TOOLTIP_CLOSE = QObject::tr("退出应用程序");
inline const QString STR_FORM_TOOLTIP_CLOSE = QObject::tr("关闭窗体");
inline const QString STR_FORM_TOOLTIP_MAX = QObject::tr("窗体复位");
inline const QString STR_FORM_TOOLTIP_NORMAL = QObject::tr("恢复窗体");
inline const QString STR_FORM_TOOLTIP_FIXED = QObject::tr("锁定窗体移动范围");

class Frameless : public QObject {
    Q_OBJECT

public:
    enum class Mode {
        MAIN,
        DIALOG,
        MESSAGEBOX,
        PANEL,
    };

    Frameless(QWidget* window = nullptr);
    virtual ~Frameless() = default;

    void init(const Mode& mode = Mode::MAIN, bool fixed = false);

    void setMainFrame(QWidget* newMainFrame);
    void setTopFrame(QWidget* newTopFrame);
    void setContentFrame(QWidget* newContentFrame);

    void setButtonMin(QToolButton* newButtonMin);
    void setButtonMax(QToolButton* newButtonMax);
    void setButtonClose(QToolButton* newButtonClose);
    void setButtonFixed(QToolButton* newButtonFixed);

    void setMainMenu(QMenuBar* newMainMenu);

Q_SIGNALS:
    void onMin();
    void onMax();

private:
    void max();

private:
    QWidget* window_;
    QString title_;

    bool fixed_ { false };
    Mode mode_ { Mode::MAIN };

    QWidget* mainFrame_ { nullptr };
    QWidget* topFrame_ { nullptr };
    QWidget* contentFrame_ { nullptr };

    QToolButton* buttonMin_ { nullptr };
    QToolButton* buttonMax_ { nullptr };
    QToolButton* buttonClose_ { nullptr };
    QToolButton* buttonFixed_ { nullptr };
    QToolButton* buttonPinned_ { nullptr };

    QMenuBar* mainMenu_ { nullptr };
};

}
#endif // FRAMELESS_H
