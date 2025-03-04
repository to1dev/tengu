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

#ifndef MNEMONICVIEW_H
#define MNEMONICVIEW_H

#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QGraphicsView>
#include <QLabel>

#include "FlowLayout.h"

namespace Daitengu::Components {

inline constexpr char VIEW_MNEMONIC[] = "viewMnemonic";
inline constexpr char LABEL_MNEMONIC[] = "labelMnemonic";

class MnemonicView : public QGraphicsView {
    Q_OBJECT

    static inline constexpr int VIEW_MARGIN = 5;

public:
    explicit MnemonicView(QWidget* parent = nullptr);
    void clear();
    void myFitInView();

    QString mnemonic() const;
    void setMnemonic(const QString& newMnemonic);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QGraphicsScene scene_;
    QGraphicsWidget* widget_ { nullptr };

    QString mnemonic_;
};

}
#endif // MNEMONICVIEW_H
