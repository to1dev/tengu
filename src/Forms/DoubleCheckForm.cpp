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

#include "DoubleCheckForm.h"
#include "ui_DoubleCheckForm.h"

DoubleCheckForm::DoubleCheckForm(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DoubleCheckForm)
{
    ui->setupUi(this);

    setWindowTitle(DEFAULT_TITLE);

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->init(Frameless::Mode::MESSAGEBOX);

    SVGWidget* icon = nullptr;
    icon = new SVGWidget(":/Emoji/5", ui->labelIcon);

    if (icon) {
        icon->setPadding(3);
        icon->setImageSize(QSize(LOGO_SIZE, LOGO_SIZE));
        ui->labelIcon->setFixedWidth(72);
    }

    connect(ui->ButtonCancel, &QPushButton::clicked, this,
        &DoubleCheckForm::reject);

    ui->labelText->setText(DOUBLE_CHECK_DELETE);
}

DoubleCheckForm::~DoubleCheckForm()
{
    delete ui;
}
