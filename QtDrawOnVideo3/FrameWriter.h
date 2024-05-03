#pragma once

#include <QVideoSink>
#include <QPointer>
#include <QVideoFrame>

class FrameWriter: public QVideoSink
{
    Q_OBJECT

public:
    explicit FrameWriter(QObject* parent = nullptr): QVideoSink(parent)
    {
        connect(this, &QVideoSink::videoFrameChanged, this, &FrameWriter::frame_handle);
    }
    ~FrameWriter() { disconnect(this, &QVideoSink::videoFrameChanged, nullptr, nullptr); }

    [[nodiscard]] QVideoSink* video_sink() const { return m_video_sink.get(); }

signals:
    void frameReady(QImage img);

private slots:
    void frame_handle(QVideoFrame const& frame)
    {
        QVideoFrame f = frame;
        if (f.isValid() && f.map(QVideoFrame::ReadOnly))
        {
            auto img = f.toImage().convertToFormat(QImage::Format_RGBA8888);
            f.unmap();

            emit frameReady(img.copy());
        }
    }

private:
    Q_DISABLE_COPY(FrameWriter)

    QPointer<QVideoSink> m_video_sink = nullptr;
};