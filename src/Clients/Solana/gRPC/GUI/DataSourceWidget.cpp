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

#include "DataSourceWidget.hpp"

#include <QVBoxLayout>

namespace solana {

DataSourceWidget::DataSourceWidget(const std::string& sourceId, QWidget* parent)
    : QWidget(parent)
    , sourceId_(sourceId)
{
    auto* layout = new QVBoxLayout(this);
    statsLabel_ = new QLabel("Waiting for data...", this);
    layout->addWidget(statsLabel_);
    setLayout(layout);

    refreshTimer_ = new QTimer(this);
    connect(refreshTimer_, &QTimer::timeout, this, [this]() {
        if (!recentData_.is_null()) {
            QString text
                = QString("Source: %1\nTransactions: %2\nFilter Hits: %3")
                      .arg(QString::fromStdString(sourceId_))
                      .arg(recentData_["transactions"].get<int>())
                      .arg(recentData_["hits"].get<int>());
            statsLabel_->setText(text);
        }
    });
    refreshTimer_->start(1000);
}

DataSourceWidget::~DataSourceWidget()
{
    refreshTimer_->stop();
}

void DataSourceWidget::updateData(
    const std::string& sourceId, const nlohmann::json& data)
{
    if (sourceId == sourceId_) {
        recentData_ = data;
    }
}
}
