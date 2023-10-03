#pragma once

#include <QObject>
#include <QSharedMemory>
#include <QSystemSemaphore>

// reference: https://stackoverflow.com/questions/5006547/qt-best-practice-for-a-single-instance-app-protection

class SingleAppGuard
{
public:
    SingleAppGuard(QString const& key);
    ~SingleAppGuard();

    bool isAppRunning();
    bool tryRun();
    void release();

private:
    QString const m_key;
    QString const m_shared_mem_key;
    QString const m_mem_lock_key;

    QSharedMemory m_shared_mem;
    QSystemSemaphore m_mem_lock;

    Q_DISABLE_COPY(SingleAppGuard)
};
