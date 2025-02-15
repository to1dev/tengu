#include "RunGuard.h"

namespace Daitengu::Utils {

RunGuard::RunGuard()
    : memLockKey_(Encryption::easyHash(QString(STR_KEY), 23))
    , sharedMemKey_(Encryption::easyHash(QString(STR_KEY), 78))
    , sharedMem_(sharedMemKey_)
    , memLock_(memLockKey_, 1)
{
    memLock_.acquire();
    {
        QSharedMemory fix(sharedMemKey_);
        fix.attach();
    }
    memLock_.release();
}

RunGuard::~RunGuard()
{
    release();
}

bool RunGuard::isRunning()
{
    if (sharedMem_.isAttached())
        return false;

    memLock_.acquire();

    const bool isRunning = sharedMem_.attach();
    if (isRunning)
        sharedMem_.detach();
    memLock_.release();

    return isRunning;
}

bool RunGuard::tryToRun()
{
    if (isRunning())
        return false;

    memLock_.acquire();

    const bool result = sharedMem_.create(sizeof(quint64));
    memLock_.release();

    if (!result) {
        release();
        return false;
    }

    return true;
}

void RunGuard::release()
{
    memLock_.acquire();
    if (sharedMem_.isAttached())
        sharedMem_.detach();
    memLock_.release();
}

}
