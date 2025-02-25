#include "GeyserClientManager.h"
#include "SwapFilter.h"

namespace Daitengu::Clients::Solana {

GeyserClientManager::GeyserClientManager(QObject* parent)
    : QObject(parent)
{
}

GeyserClientManager::~GeyserClientManager()
{
    removeAllWorkers();
}

GeyserClientWorker* GeyserClientManager::createWorker(const QString& address,
    std::vector<std::shared_ptr<TransactionFilter>> filters)
{
    auto context = std::make_unique<WorkerThreadContext>();

    GeyserClientWorker* worker
        = new GeyserClientWorker(address, std::move(filters));

    worker->moveToThread(&context->thread);

    connect(worker, &GeyserClientWorker::updateReceived, this,
        [this](const QString& msg) {
            qDebug() << "[GeyserClientManager] Update: " << msg;
        });

    connect(worker, &GeyserClientWorker::logMessage, this,
        [this](const QString& log) {
            qDebug() << "[GeyserClientManager] Log: " << log;
        });

    connect(&context->thread, &QThread::started, worker,
        &GeyserClientWorker::start);

    connect(worker, &GeyserClientWorker::finished, &context->thread,
        &QThread::quit);

    connect(
        worker, &GeyserClientWorker::finished, worker, &QObject::deleteLater);

    connect(&context->thread, &QThread::finished, this, [this, worker]() {
        qDebug() << "[GeyserClientManager] Worker thread finished.";
    });

    context->worker = worker;
    context->thread.start();

    workerThreads_.push_back(std::move(context));

    return worker;
}

void GeyserClientManager::removeWorker(GeyserClientWorker* worker)
{
    if (!worker)
        return;

    worker->stop();
    for (auto it = workerThreads_.begin(); it != workerThreads_.end(); ++it) {
        if ((*it)->worker == worker) {
            (*it)->thread.quit();
            (*it)->thread.wait();
            workerThreads_.erase(it);
            break;
        }
    }
}

void GeyserClientManager::removeAllWorkers()
{
    for (auto& ctx : workerThreads_) {
        if (ctx->worker) {
            ctx->worker->stop();
        }
        ctx->thread.quit();
        ctx->thread.wait();
    }
    workerThreads_.clear();
}

}
