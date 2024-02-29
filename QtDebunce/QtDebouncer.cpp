#include "QtDebouncer.h"

QtDebouncer::QtDebouncer(QObject* parent): QObject(parent), m_timer(this), m_has_pending(false)
{
    connect(&m_timer, &QTimer::timeout, this, &QtDebouncer::emitTriggered);
}

QtDebouncer::~QtDebouncer()
{
    m_timer.stop();
    emitTriggered();
}

int QtDebouncer::timeout() const
{
    return m_timer.interval();
}
void QtDebouncer::setTimeout(int msec)
{
    if (msec == timeout()) return;
    m_timer.setInterval(msec);
    emit timeoutChanged();
}

void QtDebouncer::debounce()
{
    m_has_pending = true;
    m_timer.start();
}

void QtDebouncer::emitTriggered()
{
    if (m_has_pending)
    {
        m_has_pending = false;
        emit triggered();
    }
}