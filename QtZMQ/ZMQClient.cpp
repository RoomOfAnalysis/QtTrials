#include "ZMQClient.h"

#include <QUuid>
#include <QSocketNotifier>

#include <zmq.h>

static bool get_rcvmore(void* sock)
{
    int more;
    size_t opt_len = sizeof(more);
    int ret = zmq_getsockopt(sock, ZMQ_RCVMORE, &more, &opt_len);
    assert(ret == 0);
    return more > 0;
}

static int get_events(void* sock)
{
    while (true)
    {
        int events;
        size_t opt_len = sizeof(events);

        int ret = zmq_getsockopt(sock, ZMQ_EVENTS, &events, &opt_len);
        if (ret == 0) return events;

        assert(errno == EINTR);
    }
}

ZMQClient::ZMQClient(QObject* parent): QObject(parent), m_id{QUuid::createUuid().toByteArray()}
{
    m_ctx = zmq_ctx_new();
    assert(m_ctx != nullptr);

    m_socket = zmq_socket(m_ctx, ZMQ_DEALER);
    assert(m_socket != nullptr);

    size_t opt_len = m_id.size();
    int ret = zmq_setsockopt(m_socket, ZMQ_IDENTITY, m_id.data(), opt_len);
    assert(ret == 0);

    qintptr fd;
    opt_len = sizeof(fd);
    ret = zmq_getsockopt(m_socket, ZMQ_FD, &fd, &opt_len);
    assert(ret == 0);

    // always check whether can read whenever read/write events fired
    m_read_notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    m_write_notifier = new QSocketNotifier(fd, QSocketNotifier::Write, this);
    connect(m_read_notifier, SIGNAL(activated(QSocketDescriptor)), this, SLOT(read_notifier_activated()));
    connect(m_write_notifier, SIGNAL(activated(QSocketDescriptor)), this, SLOT(read_notifier_activated()));
}

ZMQClient::~ZMQClient()
{
    zmq_close(m_socket);
    zmq_ctx_term(m_ctx);
}

void ZMQClient::set_address(QByteArray address)
{
    if (m_address == address) return;

    if (!m_address.isEmpty())
    {
        int ret = zmq_disconnect(m_socket, m_address.data());
        assert(ret == 0);
    }

    m_address = address;
    int ret = zmq_connect(m_socket, m_address.data());
    assert(ret == 0);
}

void ZMQClient::set_id(QByteArray id)
{
    if (m_id == id) return;

    m_id = id;
    size_t opt_len = m_id.size();
    int ret = zmq_setsockopt(m_socket, ZMQ_IDENTITY, m_id.data(), opt_len);
    assert(ret == 0);
}

bool ZMQClient::send(QList<QByteArray> const& message)
{
    for (int n = 0; n < message.count(); ++n)
    {
        const QByteArray& buf = message[n];

        zmq_msg_t msg;

        int ret = zmq_msg_init_size(&msg, buf.size());
        assert(ret == 0);

        memcpy(zmq_msg_data(&msg), buf.data(), buf.size());

        ret = zmq_msg_send(&msg, m_socket, ZMQ_DONTWAIT | (n + 1 < message.count() ? ZMQ_SNDMORE : 0));

        if (ret < 0)
        {
            ret = zmq_msg_close(&msg);
            assert(ret == 0);

            return false;
        }

        ret = zmq_msg_close(&msg);
        assert(ret == 0);
    }

    return (get_events(m_socket) & ZMQ_POLLOUT);
}
QList<QByteArray> ZMQClient::receive()
{
    QList<QByteArray> out;
    do
    {
        zmq_msg_t msg;
        int ret = zmq_msg_init(&msg);
        assert(ret == 0);

        ret = zmq_msg_recv(&msg, m_socket, ZMQ_DONTWAIT);
        if (ret < 0)
        {
            ret = zmq_msg_close(&msg);
            assert(ret == 0);

            break;
        }
        QByteArray buf((char const*)zmq_msg_data(&msg), zmq_msg_size(&msg));
        ret = zmq_msg_close(&msg);
        assert(ret == 0);

        out += buf;
    } while (get_rcvmore(m_socket));

    return out;
}

void ZMQClient::read_notifier_activated()
{
    m_read_notifier->setEnabled(false);
    m_write_notifier->setEnabled(false);
    m_can_read = (get_events(m_socket) & ZMQ_POLLIN);
    if (m_can_read) emit ready_to_read();
    m_read_notifier->setEnabled(true);
    m_write_notifier->setEnabled(true);
}