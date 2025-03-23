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

#include "MnemonicView.h"

namespace Daitengu::Components {

MnemonicView::MnemonicView(QWidget* parent)
    : QGraphicsView(parent)
    , widget_(nullptr)
{
    setViewportMargins(
        QMargins(VIEW_MARGIN, VIEW_MARGIN, VIEW_MARGIN, VIEW_MARGIN));
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    setObjectName(VIEW_MNEMONIC);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setAlignment(Qt::AlignTop | Qt::AlignLeft);
}

void MnemonicView::clear()
{
    if (scene()) {
        scene()->clear();
    }
}

void MnemonicView::myFitInView()
{
    fitInView(contentsRect(), Qt::KeepAspectRatio);
    if (scene()) {
        centerOn(scene()->width() / 2, scene()->height() / 2);
    }
}

QString MnemonicView::mnemonic() const
{
    return mnemonic_;
}

void MnemonicView::setMnemonic(const QString& newMnemonic)
{
    mnemonic_ = newMnemonic;
    scene_.clear();

    widget_ = new QGraphicsWidget;
    auto layout = new FlowLayout(widget_);
    const QStringList words = mnemonic_.split(" ", Qt::SkipEmptyParts);

    for (const auto& word : words) {
        auto* proxy = new QGraphicsProxyWidget(widget_);
        auto* label = new QLabel(word);
        label->setObjectName(LABEL_MNEMONIC);
        label->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
        proxy->setWidget(label);
        layout->addItem(proxy);
    }

    // widget_->setLayout(layout);
    scene_.addItem(widget_);
    setScene(&scene_);

    widget_->resize(contentsRect().width(), contentsRect().height());
    scene_.setSceneRect(widget_->sceneBoundingRect());
    myFitInView();
}

void MnemonicView::resizeEvent(QResizeEvent* event)
{
    if (scene() && widget_) {
        widget_->resize(contentsRect().width(), contentsRect().height());
        scene_.setSceneRect(widget_->sceneBoundingRect());
        myFitInView();
    }
    QGraphicsView::resizeEvent(event);
}

}
