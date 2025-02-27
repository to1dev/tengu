#pragma once

#include <memory>
#include <vector>

#include <QObject>
#include <QThread>

#include "GeyserClientWorker.h"

namespace Daitengu::Clients::Solana {

class GeyserClientManager : public QObject {
    Q_OBJECT

public:
    explicit GeyserClientManager(QObject* parent = nullptr);
    ~GeyserClientManager();

    GeyserClientWorker* createWorker(const QString& address,
        std::vector<std::shared_ptr<TransactionFilter>> filters = {});

    void removeWorker(GeyserClientWorker* worker);

    void removeAllWorkers();

private:
    struct WorkerThreadContext {
        QThread thread;
        GeyserClientWorker* worker = nullptr;
    };

    std::vector<std::unique_ptr<WorkerThreadContext>> workerThreads_;
};

}
