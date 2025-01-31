#include "RunGuard.h"

namespace Daitengu::Utils {

RunGuard::RunGuard()
    : mMemLockKey(Encryption::easyHash(QString(STR_KEY), 23))
    , mSharedMemKey(Encryption::easyHash(QString(STR_KEY), 78))
    , mSharedMem(mSharedMemKey)
    , mMemLock(mMemLockKey, 1)
{
    mMemLock.acquire();
    {
        QSharedMemory fix(mSharedMemKey);
        fix.attach();
    }
    mMemLock.release();
}

RunGuard::~RunGuard()
{
    release();
}

bool RunGuard::isRunning()
{
    if (mSharedMem.isAttached())
        return false;

    mMemLock.acquire();

    const bool isRunning = mSharedMem.attach();
    if (isRunning)
        mSharedMem.detach();
    mMemLock.release();

    return isRunning;
}

bool RunGuard::tryToRun()
{
    if (isRunning())
        return false;

    mMemLock.acquire();

    const bool result = mSharedMem.create(sizeof(quint64));
    mMemLock.release();

    if (!result) {
        release();
        return false;
    }

    return true;
}

void RunGuard::release()
{
    mMemLock.acquire();
    if (mSharedMem.isAttached())
        mSharedMem.detach();
    mMemLock.release();
}

}
