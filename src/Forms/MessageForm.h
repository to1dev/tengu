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

#ifndef MESSAGEFORM_H
#define MESSAGEFORM_H

#include <QDialog>

#include "Components/LineEditEx.h"
#include "Components/SVGWidget.h"

#include "Managers/WindowManager.h"

#include "UI/Frameless.h"

using namespace Daitengu::Components;
using namespace Daitengu::Core;
using namespace Daitengu::UI;

namespace Ui {
class MessageForm;
}

enum MessageButton {
    NoButton = 0x00000000,
    Ok = 0x00000400,
    Save = 0x00000800,
    Yes = 0x00004000,
    No = 0x00010000,
    Abort = 0x00040000,
    Retry = 0x00080000,
    Ignore = 0x00100000,
    Close = 0x00200000,
    Cancel = 0x00400000,
    Discard = 0x00800000,
    Help = 0x01000000,
    Apply = 0x02000000,
    Reset = 0x04000000,
};

inline constexpr char DOUBLE_DELETE_TEXT[] = "DELETE";

class MessageForm : public QDialog {
    Q_OBJECT

public:
    explicit MessageForm(QWidget* parent = nullptr, int emoji = -1,
        const QString& text = QString(),
        const QString& title = QObject::tr("Tips"), bool doubleCheck = false,
        int buttons = MessageButton::Ok);
    ~MessageForm();

protected:
    void showEvent(QShowEvent* event) override;

private Q_SLOTS:
    void ok();

private:
    Ui::MessageForm* ui;

    std::unique_ptr<Frameless> frameless_;

    LineEditEx* editDelete_;

    bool doubleCheck_ { false };
};

#endif // MESSAGEFORM_H
