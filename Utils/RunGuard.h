#ifndef RUNGUARD_H
#define RUNGUARD_H

#include <QSharedMemory>
#include <QSystemSemaphore>

#include <xxhash.h>

#include "Utils/Encryption.h"

namespace Daitengu::Utils {

inline constexpr char STR_KEY[] = "g:~V)x`6$EUPGHvh";

class RunGuard {
public:
    RunGuard();
    ~RunGuard();

    bool isRunning();
    bool tryToRun();
    void release();

private:
    const QString mMemLockKey;
    const QString mSharedMemKey;
    QSharedMemory mSharedMem;
    QSystemSemaphore mMemLock;
};

}
#endif // RUNGUARD_H
