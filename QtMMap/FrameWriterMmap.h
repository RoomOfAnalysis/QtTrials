#pragma once

#include <memory>

#ifdef DISPLAY_FPS
#include <atomic>
#endif

#include <QVideoSink>
#include <QPointer>
#include <QFile>
#include <QStandardPaths>
#include <QSystemSemaphore>

// TODO: apart from system semaphore, use fcntl flock or spsc queue (e.g. boost ipc) to sync between process?
class FrameWriterMmap: public QVideoSink
{
    Q_OBJECT

public:
    explicit FrameWriterMmap(QString filename = QStandardPaths::writableLocation(QStandardPaths::TempLocation) +
                                                "/_frame_mmap",
                             QString sem_key = "_frame_mmap_sem", QObject* parent = nullptr);
    ~FrameWriterMmap();

    void set_video_sink(QVideoSink* video_sink);

    [[nodiscard]] QVideoSink* video_sink() const;

signals:
    void video_sink_changed();

private slots:
    void frame_handle(QVideoFrame const& frame);

private:
    Q_DISABLE_COPY(FrameWriterMmap)

    QPointer<QVideoSink> m_video_sink = nullptr;

    QFile m_mmap;
    uchar* m_mmap_addr = nullptr;

    QSystemSemaphore m_sem;

#ifdef DISPLAY_FPS
    std::atomic<int> m_fps = 0;
    QTimer* m_fps_timer = nullptr;
#endif
};