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
