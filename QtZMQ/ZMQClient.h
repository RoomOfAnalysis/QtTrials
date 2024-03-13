#pragma once

#include <QObject>

class QSocketNotifier;

class ZMQClient: public QObject
{
    Q_OBJECT
public:
    explicit ZMQClient(QObject* parent = nullptr);
    ~ZMQClient();

    void set_address(QByteArray address);
    void set_id(QByteArray id);

    [[nodiscard]] QByteArray get_address() const { return m_address; }
    [[nodiscard]] QByteArray get_id() const { return m_id; }

    [[nodiscard]] bool send(QList<QByteArray> const& message);
    [[nodiscard]] QList<QByteArray> receive();

public slots:
    void read_notifier_activated();

signals:
    void ready_to_read();

private:
    QByteArray m_address{};
    QByteArray m_id{};
    QSocketNotifier* m_read_notifier = nullptr;
    QSocketNotifier* m_write_notifier = nullptr;
    void* m_ctx = nullptr;
    void* m_socket = nullptr;
    bool m_can_read = false;
};