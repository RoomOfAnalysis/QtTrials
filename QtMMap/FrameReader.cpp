#include "FrameReader.h"

#include <QVideoFrame>
#include <QBuffer>
#include <QDebug>
#include <QRandomGenerator>
#include <QPainter>
#include <QDateTime>

//#define UseDebugFrame

FrameReader::FrameReader(QString key, QObject* parent): QVideoSink(parent), m_shared_memory(key)
{
    m_timer.setInterval(33);
    connect(&m_timer, &QTimer::timeout, this, &FrameReader::timeout_handle);
}

void FrameReader::set_video_sink(QVideoSink* video_sink)
{
    if (m_video_sink == video_sink) return;

    m_video_sink = video_sink;
    emit video_sink_changed();
}

QVideoSink* FrameReader::video_sink() const
{
    return m_video_sink.get();
}

void FrameReader::start()
{
    m_timer.start();
}

void FrameReader::stop()
{
    m_timer.stop();

    m_shared_memory.detach();
}

void FrameReader::timeout_handle()
{
    if (!m_video_sink) return;

#ifdef UseDebugFrame
    QVideoFrame video_frame(QVideoFrameFormat(QSize(640, 480), QVideoFrameFormat::Format_RGBA8888));
    if (!video_frame.isValid() || !video_frame.map(QVideoFrame::WriteOnly))
    {
        qWarning() << "QVideoFrame is not valid or not writable";
        return;
    }
    QImage::Format image_format = QVideoFrameFormat::imageFormatFromPixelFormat(video_frame.pixelFormat());
    if (image_format == QImage::Format_Invalid)
    {
        qWarning() << "It is not possible to obtain image format from the pixel format of the videoframe";
        return;
    }
    int plane = 0;
    QImage image(video_frame.bits(plane), video_frame.width(), video_frame.height(), image_format);
    image.fill(QColor::fromRgb(QRandomGenerator::global()->generate()));
    QPainter painter(&image);
    painter.drawText(image.rect(), Qt::AlignCenter, QDateTime::currentDateTime().toString());
    painter.end();
#else
    if (!m_shared_memory.isAttached() && !m_shared_memory.attach())
    {
        qDebug() << "Unable to attach to shared memory segment.";
        return;
    }

    m_shared_memory.lock();

    // use QBuffer with QDataStream is very very slow...
    //QBuffer buffer;
    //QDataStream in(&buffer);
    //QImage image;
    //buffer.setData((char*)m_shared_memory.constData(), m_shared_memory.size());
    //buffer.open(QBuffer::ReadOnly);
    //in >> image;

    uchar* from = (uchar*)m_shared_memory.data();

    int sizeInBytes = *(int*)from;
    from += sizeof(int);

    //Get the width of the image and move the pointer forward
    int width = *(int*)from;
    from += sizeof(int);

    //Get the height of the image and move the pointer forward
    int height = *(int*)from;
    from += sizeof(int);

    //Get the image format of the image and move the pointer forward
    int imageFormat = *(int*)from;
    from += sizeof(int);

    //Get the bytes per line of the image and move the pointer forward
    int bytesPerLine = *(int*)from;
    from += sizeof(int);

    //Generate an image using the raw data and move the pointer forward
    QImage image(from, width, height, bytesPerLine, (QImage::Format)imageFormat);

    m_shared_memory.unlock();

    QVideoFrame video_frame(
        QVideoFrameFormat(image.size(), QVideoFrameFormat::pixelFormatFromImageFormat(image.format())));
    if (!video_frame.isValid() || !video_frame.map(QVideoFrame::WriteOnly))
    {
        qWarning() << "QVideoFrame is not valid or not writable";
        return;
    }
    memcpy(video_frame.bits(0), image.bits(), image.sizeInBytes());
#endif

    video_frame.unmap();
    m_video_sink->setVideoFrame(video_frame);
}