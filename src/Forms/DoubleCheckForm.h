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

#ifndef DOUBLECHECKFORM_H
#define DOUBLECHECKFORM_H

#include <QDialog>

#include "Components/SVGWidget.h"
#include "Managers/WindowManager.h"

#include "UI/Frameless.h"

using namespace Daitengu::Components;
using namespace Daitengu::Core;
using namespace Daitengu::UI;

namespace Ui {
class DoubleCheckForm;
}

inline const QString DOUBLE_CHECK_DELETE = QObject::tr(
    "<font style='color:red'>警告！</font><p>你即将永久删除 "
    "这个钱包，此操作无法撤销！</p><p>注意："
    "一旦删除，如果你未备份助记词或私钥，该钱包将永远消失，无法找回！你确定要这"
    "么做吗？</p><p>请在下方输入大写 <strong "
    "style='color:olive'>DELETE</strong> 以确认：</p>");

class DoubleCheckForm : public QDialog {
    Q_OBJECT

    const QString DEFAULT_TITLE = QObject::tr("Double Check: Confirm Deletion");

public:
    explicit DoubleCheckForm(QWidget* parent = nullptr);
    ~DoubleCheckForm();

private:
    Ui::DoubleCheckForm* ui;

    std::unique_ptr<Frameless> frameless_;
};

#endif // DOUBLECHECKFORM_H
