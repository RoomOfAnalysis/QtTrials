#pragma once

#include <memory>

#ifdef DISPLAY_FPS
#include <atomic>
#endif

#include <QVideoSink>
#include <QPointer>
#include <QLocalServer>
#include <QMutex>

class FrameWriterSocket: public QVideoSink
{
    Q_OBJECT

public:
    explicit FrameWriterSocket(QString server_name = "_frame_mmap", QObject* parent = nullptr);
    ~FrameWriterSocket() = default;

    void set_video_sink(QVideoSink* video_sink);

    [[nodiscard]] QVideoSink* video_sink() const;

signals:
    void video_sink_changed();

private slots:
    void frame_handle(QVideoFrame const& frame);

private:
    Q_DISABLE_COPY(FrameWriterSocket)

    QPointer<QVideoSink> m_video_sink = nullptr;

    QLocalServer* m_server = nullptr;

    QMutex m_mtx;
    QByteArray m_buf;

#ifdef DISPLAY_FPS
    std::atomic<int> m_fps = 0;
    QTimer* m_fps_timer = nullptr;
#endif
};