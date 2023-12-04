#include "QtFrameHandler.h"

#include <QVideoFrame>
#include <QTimer>
#include <QDebug>

QtFrameHandler::QtFrameHandler(QObject* parent): QVideoSink(parent)
{
    connect(this, &QVideoSink::videoFrameChanged, this, &QtFrameHandler::frame_handle);

    m_fps_timer = new QTimer(this);
    m_fps_timer->setInterval(1000); // 1s
    connect(m_fps_timer, &QTimer::timeout, this, [this]() {
        qDebug() << "fps:" << m_fps;
        m_fps = 0;
    });
    m_fps_timer->start();
}

void QtFrameHandler::setVideoSink(QVideoSink* sink)
{
    if (m_video_sink != sink) m_video_sink = sink;
}

QVideoSink* QtFrameHandler::videoSink() const
{
    return m_video_sink.get();
}

void QtFrameHandler::setFrameProcessor(std::function<void(QImage*)> processor)
{
    m_frame_processor = processor;
}

void QtFrameHandler::frame_handle(QVideoFrame const& frame)
{
    // meawhile, its memory usage is still very high
    QVideoFrame f = frame; // shallow copy
    if (m_video_sink && f.isValid() && f.map(QVideoFrame::ReadOnly))
    {
        auto img = f.toImage().convertToFormat(QImage::Format_RGBA8888);
        f.unmap();

        if (m_frame_processor) m_frame_processor(&img);

        if (auto new_frame =
                QVideoFrame(QVideoFrameFormat(img.size(), QVideoFrameFormat::pixelFormatFromImageFormat(img.format())));
            new_frame.isValid() && new_frame.map(QVideoFrame::WriteOnly))
        {
            memcpy(new_frame.bits(0), img.bits(), img.sizeInBytes());
            new_frame.unmap();

            m_video_sink->setVideoFrame(new_frame);
            m_fps++;
        }
    }
}