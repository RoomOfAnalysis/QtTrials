#pragma once

#include <memory>

#include <QVideoSink>
#include <QPointer>
#include <QSharedMemory>

class FrameWriter: public QVideoSink
{
    Q_OBJECT

public:
    explicit FrameWriter(QString key = "_frame_mmap", QObject* parent = nullptr);
    ~FrameWriter() = default;

    void set_video_sink(QVideoSink* video_sink);

    [[nodiscard]] QVideoSink* video_sink() const;

signals:
    void video_sink_changed();

private slots:
    void frame_handle(QVideoFrame const& frame);

private:
    Q_DISABLE_COPY(FrameWriter)

    QPointer<QVideoSink> m_video_sink = nullptr;

    QSharedMemory m_shared_memory;
};