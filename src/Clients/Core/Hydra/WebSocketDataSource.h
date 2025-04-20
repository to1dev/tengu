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

#include <functional>

#include <QMutex>
#include <QStringList>
#include <QTimer>
#include <QWebSocket>

#include <qcoro/QCoro>
#include <qcoro/QCoroWebSocket>

#include "DataSource.h"

namespace Daitengu::Clients::Hydra {

class WebSocketDataSource : public DataSource {
    Q_OBJECT

public:
    struct Config {
        QString messageType;
        QString symbolField;
        QString priceField;
        std::function<QString(const QString&)> formatSymbol;
    };

    explicit WebSocketDataSource(const QString& name, const QString& wsUrl,
        const Config& config, QObject* parent = nullptr);
    ~WebSocketDataSource() override;

    void start() override;
    void stop() override;
    QVariantMap getData() const override;

    QString getName() const override
    {
        return name_;
    }

    void setSymbolList(const QStringList& symbols);
    void setBatchInterval(int ms);

private:
    QCoro::Task<void> connectWebSocket();
    void processMessage(const QString& message);
    void subscribeToSymbols();
    void sendPing();
    void batchUpdate();
    void scheduleReconnect(int delayMs);

    QString name_;
    QString wsUrl_;
    Config config_;
    bool running_ { false };
    QStringList symbolList_;
    std::unique_ptr<QWebSocket> webSocket_;
    QTimer* pingTimer_ { nullptr };
    QTimer* batchTimer_ { nullptr };
    QTimer* connectTimeoutTimer_ { nullptr };

    mutable QMutex dataMutex_;
    QVariantMap prices_;
    QVariantMap pendingPrices_;                          // 批量更新的临时存储
    int reconnectAttempts_ { 0 };
    static constexpr int MAX_RECONNECT_ATTEMPTS = 10;
    static constexpr int MAX_RECONNECT_DELAY_MS = 32000; // 32 秒
    static constexpr int CONNECT_TIMEOUT_MS = 10000;     // 10 秒
};
}
