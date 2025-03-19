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

#include "MessageForm.h"
#include "ui_MessageForm.h"

MessageForm::MessageForm(QWidget* parent, int emoji, const QString& text,
    const QString& title, bool doubleCheck, int buttons)
    : QDialog(parent)
    , ui(new Ui::MessageForm)
    , doubleCheck_(doubleCheck)
{
    ui->setupUi(this);

    setWindowTitle(title);

    frameless_ = std::make_unique<Frameless>(this);
    frameless_->setMainFrame(ui->frameMain);
    frameless_->setContentFrame(ui->frameContent);
    frameless_->init(Frameless::Mode::MESSAGEBOX);

    SVGWidget* icon = nullptr;
    if (emoji > 0) {
        icon = new SVGWidget(QString(":/Emoji/%1").arg(emoji), ui->labelIcon);
    } else {
        icon = new SVGWidget(
            // TODO
            QString(":/Emoji/%1").arg(randomIndex(1, 20)), ui->labelIcon);
    }
    if (icon) {
        icon->setPadding(3);
        icon->setImageSize(QSize(LOGO_SIZE, LOGO_SIZE));
        ui->labelIcon->setFixedWidth(72);
    }

    ui->labelText->setText(text);

    if (buttons & MessageButton::Ok) {
        connect(ui->ButtonOK, &QPushButton::clicked, this, &MessageForm::ok);
    } else {
        ui->ButtonOK->setVisible(false);
    }

    if (buttons & MessageButton::Cancel) {
        connect(ui->ButtonCancel, &QPushButton::clicked, this,
            &MessageForm::reject);
    } else {
        ui->ButtonCancel->setVisible(false);
    }
}

MessageForm::~MessageForm()
{
    delete ui;
}

void MessageForm::ok()
{
    if (doubleCheck_) {
        DoubleCheckForm dcf(this);
        if (dcf.exec()) {
        } else {
        }
    } else {
        accept();
    }
}
