#pragma once

#include <memory>

#include <QVideoSink>
#include <QPointer>
#include <QSharedMemory>
#include <QTimer>

class FrameReader: public QVideoSink
{
    Q_OBJECT

public:
    explicit FrameReader(QString key = "_frame_mmap", QObject* parent = nullptr);
    ~FrameReader() = default;

    void set_video_sink(QVideoSink* video_sink);

    [[nodiscard]] QVideoSink* video_sink() const;

    void start();
    void stop();

signals:
    void video_sink_changed();

private slots:
    void timeout_handle();

private:
    Q_DISABLE_COPY(FrameReader)

    QSharedMemory m_shared_memory;
    QPointer<QVideoSink> m_video_sink = nullptr;

    QTimer m_timer;
};