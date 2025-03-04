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

#ifndef GEYSERCLIENTWORKER_H
#define GEYSERCLIENTWORKER_H

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QString>
#include <QThread>

#include <grpcpp/grpcpp.h>

#include "TransactionFilter.h"

namespace Daitengu::Clients::Solana {

class GeyserClientWorker : public QObject {
    Q_OBJECT

public:
    explicit GeyserClientWorker(const QString& address,
        std::vector<std::shared_ptr<TransactionFilter>> filters = {},
        QObject* parent = nullptr);

public Q_SLOTS:
    void start();
    void stop();

Q_SIGNALS:
    void updateReceived(const QString& update);
    void logMessage(const QString& log);
    void finished();

private:
    void runConnection();
    void cancelCurrentContext();

    void handleUpdate(const geyser::SubscribeUpdate& update,
        grpc::ClientReaderWriter<geyser::SubscribeRequest,
            geyser::SubscribeUpdate>* stream);

private:
    QString address_;
    std::atomic_bool shouldRun_;
    QMutex contextMutex_;
    grpc::ClientContext* context_ { nullptr };

    std::vector<std::shared_ptr<TransactionFilter>> filters_;
};

}
#endif // GEYSERCLIENTWORKER_H
