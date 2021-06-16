#include "detector.h"
#if _pragma_once_support
#pragma once
#endif
#ifndef RUNGUARD_H
#define RUNGUARD_H

/*
 * Author: Dmitry Sazonov
 * Link:
 * https://stackoverflow.com/questions/5006547/qt-best-practice-for-a-single-instance-app-protection
 */

#include <QSharedMemory>
#include <QSystemSemaphore>

//
class RunGuard final
{
    Q_DISABLE_COPY(RunGuard)

public:
    RunGuard(const QString &key);
    ~RunGuard();

    bool isAnotherRunning();
    bool tryToRun();
    void release();

private:
    const QString key;
    const QString memLockKey;
    const QString sharedmemKey;

    QSharedMemory sharedMem;
    QSystemSemaphore memLock;
};

#endif // RUNGUARD_H
