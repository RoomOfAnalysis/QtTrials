#pragma once

#include <QObject>
#include <QTimer>

class QtDebouncer: public QObject
{
    Q_OBJECT
    Q_PROPERTY(int timeout READ timeout WRITE setTimeout NOTIFY timeoutChanged)

public:
    explicit QtDebouncer(QObject* parent = nullptr);
    ~QtDebouncer();

    int timeout() const;
    void setTimeout(int msec);

signals:
    void triggered();
    void timeoutChanged();

public slots:
    void debounce();

private:
    void emitTriggered();

private:
    QTimer m_timer;
    bool m_has_pending;
};