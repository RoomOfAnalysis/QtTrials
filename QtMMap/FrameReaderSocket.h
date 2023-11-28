#pragma once

#include <memory>

#include <QVideoSink>
#include <QPointer>
#include <QLocalSocket>
#include <QTimer>

class FrameReaderSocket: public QVideoSink
{
    Q_OBJECT

public:
    explicit FrameReaderSocket(QString server_name = "_frame_mmap", QObject* parent = nullptr);
    ~FrameReaderSocket()
    {
        m_socket.disconnectFromServer();
        m_timer.stop();
    }

    void set_video_sink(QVideoSink* video_sink);

    [[nodiscard]] QVideoSink* video_sink() const;

    void start();
    void stop();

signals:
    void video_sink_changed();

private slots:
    void timeout_handle();

private:
    Q_DISABLE_COPY(FrameReaderSocket)

    QLocalSocket m_socket;
    QPointer<QVideoSink> m_video_sink = nullptr;

    QTimer m_timer;
};