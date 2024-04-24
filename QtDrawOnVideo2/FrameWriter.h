#pragma once

#include <QVideoSink>
#include <QPointer>
#include <QVideoFrame>

#include "ImageViewer.h"

class FrameWriter: public QVideoSink
{
    Q_OBJECT

public:
    explicit FrameWriter(QObject* parent = nullptr): QVideoSink(parent)
    {
        connect(this, &QVideoSink::videoFrameChanged, this, &FrameWriter::frame_handle);
    }
    ~FrameWriter() { disconnect(this, &QVideoSink::videoFrameChanged, nullptr, nullptr); }

    void set_image_viewer(ImageViewer* img_viewer) { m_image_viewer = img_viewer; }

    [[nodiscard]] QVideoSink* video_sink() const { return m_video_sink.get(); }

private slots:
    void frame_handle(QVideoFrame const& frame)
    {
        QVideoFrame f = frame;
        if (f.isValid() && f.map(QVideoFrame::ReadOnly))
        {
            auto img = f.toImage().convertToFormat(QImage::Format_RGBA8888);
            f.unmap();

            // display
            if (m_image_viewer) m_image_viewer->setImage(img);
        }
    }

private:
    Q_DISABLE_COPY(FrameWriter)

    QPointer<QVideoSink> m_video_sink = nullptr;
    ImageViewer* m_image_viewer = nullptr;
};