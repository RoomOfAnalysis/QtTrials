#pragma once

#include <memory>

#include <QVideoSink>
#include <QPointer>
#include <QFile>
#include <QStandardPaths>
#include <QTimer>
#include <QSystemSemaphore>

class FrameReaderMmap: public QVideoSink
{
    Q_OBJECT

public:
    explicit FrameReaderMmap(QString filename = QStandardPaths::writableLocation(QStandardPaths::TempLocation) +
                                                "/_frame_mmap",
                             QString sem_key = "_frame_mmap_sem", QObject* parent = nullptr);
    ~FrameReaderMmap();

    void set_video_sink(QVideoSink* video_sink);

    [[nodiscard]] QVideoSink* video_sink() const;

    void start();
    void stop();

signals:
    void video_sink_changed();

private slots:
    void timeout_handle();

private:
    Q_DISABLE_COPY(FrameReaderMmap)

    QPointer<QVideoSink> m_video_sink = nullptr;

    QTimer m_timer;

    QFile m_mmap;
    uchar* m_mmap_addr = nullptr;

    QSystemSemaphore m_sem;
};