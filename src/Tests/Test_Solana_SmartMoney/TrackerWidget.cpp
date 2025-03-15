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

#include "TrackerWidget.h"

TrackerWidget::TrackerWidget(QWidget* parent)
    : QWidget(parent)
{
    initUI();

    connect(&tracker_, &SmartMoneyTracker::smartMoneyTransactionDetected, this,
        &TrackerWidget::onTransactionDetected);

    connect(&tracker_, &SmartMoneyTracker::trackingStatusChanged, this,
        &TrackerWidget::updateUIState);

    connect(&tracker_, &SmartMoneyTracker::error, [this](const QString& error) {
        QMessageBox::warning(this, "Error", error);
    });
}

TrackerWidget::~TrackerWidget()
{
}

void TrackerWidget::setTrackerName(const QString& name)
{
    tracker_.setName(name);
    setWindowTitle("Tracker: " + name);
}

json TrackerWidget::saveConfigToJson() const
{
    json config;

    config["name"] = tracker_.getName().toStdString();
    config["minAmount"] = minAmountSpin_->value() * 1000000000ULL;

    json addresses = json::array();
    for (const QString& address :
        tracker_.getCurrentCriteria().smartAddresses) {
        addresses.push_back(address.toStdString());
    }
    config["smartAddresses"] = addresses;

    json programIds = json::array();
    for (const QString& programId :
        tracker_.getCurrentCriteria().trackedProgramIds) {
        programIds.push_back(programId.toStdString());
    }
    config["programIds"] = programIds;

    return config;
}

void TrackerWidget::loadConfigFromJson(const json& config)
{
    if (tracker_.isTracking()) {
        tracker_.stopTracking();
    }

    if (config.contains("name")) {
        setTrackerName(
            QString::fromStdString(config["name"].get<std::string>()));
    }

    if (config.contains("minAmount")) {
        uint64_t lamports = config["minAmount"].get<uint64_t>();
        minAmountSpin_->setValue(static_cast<int>(lamports / 1000000000ULL));
    }

    addressesTable_->setRowCount(0);

    if (config.contains("smartAddresses")
        && config["smartAddresses"].is_array()) {
        for (const auto& address : config["smartAddresses"]) {
            QString addrStr
                = QString::fromStdString(address.get<std::string>());

            int row = addressesTable_->rowCount();
            addressesTable_->insertRow(row);
            addressesTable_->setItem(row, 0, new QTableWidgetItem(addrStr));

            tracker_.addSmartMoneyAddress(addrStr);
        }
    }

    programIdsTable_->setRowCount(0);

    if (config.contains("programIds") && config["programIds"].is_array()) {
        for (const auto& programId : config["programIds"]) {
            QString idStr
                = QString::fromStdString(programId.get<std::string>());

            int row = programIdsTable_->rowCount();
            programIdsTable_->insertRow(row);
            programIdsTable_->setItem(row, 0, new QTableWidgetItem(idStr));

            tracker_.addTrackedProgramId(idStr);
        }
    }

    applyConfig();
    updateUIState();
}

void TrackerWidget::onStartStopClicked()
{
}

void TrackerWidget::onAddAddressClicked()
{
}

void TrackerWidget::onRemoveAddressClicked()
{
}

void TrackerWidget::onAddProgramIdClicked()
{
}

void TrackerWidget::onRemoveProgramIdClicked()
{
}

void TrackerWidget::onMinAmountChanged(int value)
{
}

void TrackerWidget::onTransactionDetected(const json& transaction)
{
}

void TrackerWidget::onClearTransactionsClicked()
{
}

void TrackerWidget::onExportTransactionsClicked()
{
}

void TrackerWidget::initUI()
{
    mainLayout_ = new QVBoxLayout(this);
}

void TrackerWidget::updateUIState()
{
}

void TrackerWidget::addTransactionToTable(const json& transaction)
{
}

void TrackerWidget::applyConfig()
{
}
