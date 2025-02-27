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
