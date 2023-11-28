#include "FrameWriterSocket.h"

#include <QVideoFrame>
#include <QLocalSocket>
#include <QDebug>

#ifdef DISPLAY_FPS
#include <QTimer>
#endif

FrameWriterSocket::FrameWriterSocket(QString server_name, QObject* parent): QVideoSink(parent)
{
    m_buf.resize(static_cast<qsizetype>(1920 * 1080 * 4) * 2);
    m_server = new QLocalServer(this);
    m_server->setMaxPendingConnections(1);
    if (!m_server->listen(server_name))
    {
        qDebug() << "Failed to start server";
        return;
    }

    connect(m_server, &QLocalServer::newConnection, [this]() {
        QLocalSocket* socket = m_server->nextPendingConnection();
        qDebug() << "new connection:" << socket;
        connect(socket, &QLocalSocket::readyRead, [this, socket]() {
            QString message = socket->readAll();
            // qDebug() << "server recv:" << message;
            if (message == "rdy")
            {
                m_mtx.lock();
                if (!m_buf.isEmpty())
                {
                    socket->write(m_buf);
                    // https://doc.qt.io/qt-6/qlocalsocket.html#flush
                    // qDebug() << "frame sent:" << socket->flush();
                }
                m_mtx.unlock();
            }
        });
        connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
    });

    connect(this, &QVideoSink::videoFrameChanged, this, &FrameWriterSocket::frame_handle);

#ifdef DISPLAY_FPS
    m_fps_timer = new QTimer(this);
    m_fps_timer->setInterval(1000); // 1s
    connect(m_fps_timer, &QTimer::timeout, this, [this]() {
        qDebug() << "fps:" << m_fps;
        m_fps = 0;
    });
    m_fps_timer->start();
#endif
}

void FrameWriterSocket::set_video_sink(QVideoSink* video_sink)
{
    if (m_video_sink == video_sink) return;

    m_video_sink = video_sink;
    emit video_sink_changed();
}

QVideoSink* FrameWriterSocket::video_sink() const
{
    return m_video_sink.get();
}

void FrameWriterSocket::frame_handle(QVideoFrame const& frame)
{
    QVideoFrame f = frame; // shallow copy
    if (f.isValid() && f.map(QVideoFrame::ReadOnly))
    {
        auto img = f.toImage().convertToFormat(QImage::Format_RGBA8888);
        f.unmap();

        m_mtx.lock();
        char* to = (char*)m_buf.data();

        //Write the total size of the image and move the pointer forward
        int sizeInBytes = img.sizeInBytes();
        auto* from = (uchar*)&sizeInBytes;
        memcpy(to, from, sizeof(int));
        to += sizeof(int);

        //Write the width of the image and move the pointer forward
        int width = img.width();
        from = (uchar*)&width;
        memcpy(to, from, sizeof(int));
        to += sizeof(int);

        //Write the height of the image and move the pointer forward
        int height = img.height();
        from = (uchar*)&height;
        memcpy(to, from, sizeof(int));
        to += sizeof(int);

        //Write the image format of the image and move the pointer forward
        int imageFormat = (int)img.format();
        from = (uchar*)&imageFormat;
        memcpy(to, from, sizeof(int));
        to += sizeof(int);

        //Write the bytes per line of the image and move the pointer forward
        int bytesPerLine = img.bytesPerLine();
        from = (uchar*)&bytesPerLine;
        memcpy(to, from, sizeof(int));
        to += sizeof(int);

        //Write the raw data of the image and move the pointer forward
        from = (uchar*)img.bits();
        memcpy(to, from, sizeInBytes);
        to += sizeInBytes;

        //qDebug() << (void*)m_buf.data() << (void*)to << (void*)(to - m_buf.data()) << m_buf.size();  //9'216'020 16'588'800
        m_mtx.unlock();
    }
    if (m_video_sink) m_video_sink->setVideoFrame(frame);

#ifdef DISPLAY_FPS
    m_fps++;
#endif
}