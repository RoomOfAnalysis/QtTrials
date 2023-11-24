#include "FrameReaderMmap.h"

#include <QVideoFrame>
#include <QDebug>

FrameReaderMmap::FrameReaderMmap(QString filename, QString sem_key, QObject* parent)
    : QVideoSink(parent), m_mmap(filename), m_sem(sem_key, 1, QSystemSemaphore::Open)
{
    if (!m_mmap.isOpen() && !m_mmap.open(QIODevice::ReadOnly))
    {
        qDebug() << "Unable to open mmap file" << m_mmap.fileName();
        return;
    }

    m_mmap_addr = (uchar*)m_mmap.map(0, m_mmap.size());

    m_timer.setInterval(33);
    connect(&m_timer, &QTimer::timeout, this, &FrameReaderMmap::timeout_handle);
}

FrameReaderMmap::~FrameReaderMmap()
{
    m_sem.release();

    m_mmap.unmap(m_mmap_addr);
    m_mmap.close();
    m_mmap_addr = nullptr;
}

void FrameReaderMmap::set_video_sink(QVideoSink* video_sink)
{
    if (m_video_sink == video_sink) return;

    m_video_sink = video_sink;
    emit video_sink_changed();
}

QVideoSink* FrameReaderMmap::video_sink() const
{
    return m_video_sink.get();
}

void FrameReaderMmap::start()
{
    m_timer.start();
}

void FrameReaderMmap::stop()
{
    m_timer.stop();
}

void FrameReaderMmap::timeout_handle()
{
    if (!m_video_sink) return;

    m_sem.acquire();
    QByteArray data;
    uchar* from = m_mmap_addr;
    if (!from)
    {
        data = m_mmap.readAll();
        from = (uchar*)data.constData();
        qDebug() << "Fallback to read file into memory";
    }

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

    QVideoFrame video_frame(
        QVideoFrameFormat(image.size(), QVideoFrameFormat::pixelFormatFromImageFormat(image.format())));
    if (!video_frame.isValid() || !video_frame.map(QVideoFrame::WriteOnly))
    {
        qWarning() << "QVideoFrame is not valid or not writable";
        return;
    }
    memcpy(video_frame.bits(0), image.bits(), image.sizeInBytes());

    m_sem.release();

    video_frame.unmap();
    m_video_sink->setVideoFrame(video_frame);
}