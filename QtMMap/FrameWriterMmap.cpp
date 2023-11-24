#include "FrameWriterMmap.h"

#include <QVideoFrame>
#include <QBuffer>
#include <QDebug>

FrameWriterMmap::FrameWriterMmap(QString filename, QString sem_key, QObject* parent)
    : QVideoSink(parent), m_mmap(filename), m_sem(sem_key, 1, QSystemSemaphore::Create)
{
    if (!m_mmap.isOpen() && !m_mmap.open(QIODevice::ReadWrite))
        qDebug() << tr("Unable to open mmap: %1").arg(m_mmap.errorString());
    qDebug() << "mmap resize" << m_mmap.resize(static_cast<qint64>(1920 * 1080 * 4) * 2);
    m_mmap_addr = m_mmap.map(0, m_mmap.size());
    connect(this, &QVideoSink::videoFrameChanged, this, &FrameWriterMmap::frame_handle);
}

FrameWriterMmap::~FrameWriterMmap()
{
    m_sem.release();

    m_mmap.unmap(m_mmap_addr);
    m_mmap.close();
    m_mmap.remove(); // delete file
    m_mmap_addr = nullptr;
}

void FrameWriterMmap::set_video_sink(QVideoSink* video_sink)
{
    if (m_video_sink == video_sink) return;

    m_video_sink = video_sink;
    emit video_sink_changed();
}

QVideoSink* FrameWriterMmap::video_sink() const
{
    return m_video_sink.get();
}

void FrameWriterMmap::frame_handle(QVideoFrame const& frame)
{
    QVideoFrame f = frame; // shallow copy
    if (f.isValid() && f.map(QVideoFrame::ReadOnly))
    {
        auto img = f.toImage().convertToFormat(QImage::Format_RGBA8888);
        f.unmap();

        m_sem.acquire();

        uchar* to = m_mmap_addr;

        //Write the total size of the image and move the pointer forward
        int sizeInBytes = img.sizeInBytes();
        auto* from = (uchar*)&sizeInBytes;
        memcpy(to, from, sizeof(int));
        to += sizeof(int);

        //Write the width of the image and move the pointer forward
        int width = img.width();
        from = (uchar*)&width;
        memcpy(to, from, sizeof(int));
        to += sizeof(int);

        //Write the height of the image and move the pointer forward
        int height = img.height();
        from = (uchar*)&height;
        memcpy(to, from, sizeof(int));
        to += sizeof(int);

        //Write the image format of the image and move the pointer forward
        int imageFormat = (int)img.format();
        from = (uchar*)&imageFormat;
        memcpy(to, from, sizeof(int));
        to += sizeof(int);

        //Write the bytes per line of the image and move the pointer forward
        int bytesPerLine = img.bytesPerLine();
        from = (uchar*)&bytesPerLine;
        memcpy(to, from, sizeof(int));
        to += sizeof(int);

        //Write the raw data of the image and move the pointer forward
        from = (uchar*)img.bits();
        memcpy(to, from, sizeInBytes);
        to += sizeInBytes;

        m_sem.release();
    }
    if (m_video_sink) m_video_sink->setVideoFrame(frame);
}