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

#pragma once

#include <QLabel>
#include <QTimer>
#include <QWidget>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace solana {

class DataSourceWidget : public QWidget {
    Q_OBJECT

public:
    explicit DataSourceWidget(
        const std::string& sourceId, QWidget* parent = nullptr);
    ~DataSourceWidget();

    std::string getSourceId() const
    {
        return sourceId_;
    }

public Q_SLOTS:
    void updateData(const std::string& sourceId, const nlohmann::json& data);

private:
    std::string sourceId_;
    QLabel* statsLabel_;
    QTimer* refreshTimer_;
    json recentData_;
};
}
