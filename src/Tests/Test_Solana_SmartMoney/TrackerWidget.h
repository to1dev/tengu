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

#include <fstream>

#include <QComboBox>
#include <QDateTime>
#include <QFileDialog>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include "Clients/Solana/WebSocket/SmartMoneyTracker.h"

using namespace Daitengu::Clients::Solana;

class TrackerWidget : public QWidget {
    Q_OBJECT

public:
    explicit TrackerWidget(QWidget* parent = nullptr);
    ~TrackerWidget();

    void setTrackerName(const QString& name);

    json saveConfigToJson() const;

    void loadConfigFromJson(const json& config);

Q_SIGNALS:
    void smartMoneyDetected(
        const QString& trackerName, const json& transaction);

private Q_SLOTS:
    void onStartStopClicked();
    void onAddAddressClicked();
    void onRemoveAddressClicked();
    void onAddProgramIdClicked();
    void onRemoveProgramIdClicked();
    void onMinAmountChanged(int value);
    void onTransactionDetected(const json& transaction);
    void onClearTransactionsClicked();
    void onExportTransactionsClicked();

private:
    void initUI();
    void updateUIState();
    void addTransactionToTable(const json& transaction);
    void applyConfig();

private:
    QVBoxLayout* mainLayout_;

    QGroupBox* configGroup_;
    QVBoxLayout* configLayout_;

    QLabel* addressLabel_;
    QLineEdit* addressInput_;
    QPushButton* addAddressBtn_;
    QTableWidget* addressesTable_;
    QPushButton* removeAddressBtn_;

    QLabel* programIdLabel_;
    QLineEdit* programIdInput_;
    QPushButton* addProgramIdBtn_;
    QTableWidget* programIdsTable_;
    QPushButton* removeProgramIdBtn_;

    QLabel* minAmountLabel_;
    QSpinBox* minAmountSpin_;

    QPushButton* startStopBtn_;

    QLabel* transactionsLabel_;
    QTableWidget* transactionsTable_;
    QPushButton* clearTransactionsBtn_;
    QPushButton* exportTransactionsBtn_;

    QLabel* statusLabel_;

    SmartMoneyTracker tracker_;

    QList<json> detectedTransactions_;
};
