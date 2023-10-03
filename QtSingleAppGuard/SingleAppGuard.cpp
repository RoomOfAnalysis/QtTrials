#include "SingleAppGuard.h"

#include <QCryptographicHash>

QString gen_key_hash(QString const& key, QString const& salt)
{
    QByteArray hash;
    hash.append(key.toUtf8());
    hash.append(salt.toUtf8());
    hash = QCryptographicHash::hash(hash, QCryptographicHash::Sha1).toHex();
    return hash;
}

SingleAppGuard::SingleAppGuard(QString const& key)
    : m_key(key), m_shared_mem_key(gen_key_hash(key, "_mem_shared_key")),
      m_mem_lock_key(gen_key_hash(key, "_mem_lock_key")), m_shared_mem(m_shared_mem_key), m_mem_lock(m_mem_lock_key, 1)
{
    m_mem_lock.acquire();
    {
        QSharedMemory fix(m_shared_mem_key); // fix for *nix
        fix.attach();
    }
    m_mem_lock.release();
}

SingleAppGuard::~SingleAppGuard()
{
    release();
}

bool SingleAppGuard::isAppRunning()
{
    if (m_shared_mem.isAttached()) return false;

    m_mem_lock.acquire();
    bool is_running = m_shared_mem.attach();
    if (is_running) m_shared_mem.detach();
    m_mem_lock.release();

    return is_running;
}

bool SingleAppGuard::tryRun()
{
    if (isAppRunning()) return false;

    m_mem_lock.acquire();
    bool ok = m_shared_mem.create(sizeof(quint64));
    m_mem_lock.release();
    if (!ok)
    {
        release();
        return false;
    }

    return true;
}

void SingleAppGuard::release()
{
    m_mem_lock.acquire();
    if (m_shared_mem.isAttached()) m_shared_mem.detach();
    m_mem_lock.release();
}