#include "FrameWriter.h"

#include <QVideoFrame>
#include <QBuffer>
#include <QDebug>

FrameWriter::FrameWriter(QString key, QObject* parent): QVideoSink(parent), m_shared_memory(key)
{
    connect(this, &QVideoSink::videoFrameChanged, this, &FrameWriter::frame_handle);
    qDebug() << (m_shared_memory.create(sizeof(char) * 1920 * 1080 * 4 * 10) ? "successfully" : "failed to")
             << "create shared memory with key: " << key << "with nativeKey:" << m_shared_memory.nativeKey();
}

void FrameWriter::set_video_sink(QVideoSink* video_sink)
{
    if (m_video_sink == video_sink) return;

    m_video_sink = video_sink;
    emit video_sink_changed();
}

QVideoSink* FrameWriter::video_sink() const
{
    return m_video_sink.get();
}

void FrameWriter::frame_handle(QVideoFrame const& frame)
{
    QVideoFrame f = frame; // shallow copy
    if (f.isValid() && f.map(QVideoFrame::ReadOnly))
    {
        auto img = f.toImage().convertToFormat(QImage::Format_RGBA8888);
        f.unmap();

        /*if (!m_shared_memory.create(size))
        {
            if (m_shared_memory.error() == QSharedMemory::AlreadyExists)
                m_shared_memory.attach();
            else
                qDebug()
                    << tr("Unable to create or attach to shared memory segment: %1").arg(m_shared_memory.errorString());
        }*/

        if (!m_shared_memory.isAttached() && !m_shared_memory.attach())
            qDebug() << tr("Unable to attach to shared memory segment: %1").arg(m_shared_memory.errorString());
        else
        {
            // use QBuffer with QDataStream is very very slow...
            //QBuffer buffer;
            //buffer.open(QBuffer::Truncate);
            //QDataStream out(&buffer);
            //out << img;
            //int size = buffer.size();

            //m_shared_memory.lock();

            //char* to = (char*)m_shared_memory.data();
            //const char* from = buffer.data().data();
            //memcpy(to, from, size);

            char* to = (char*)m_shared_memory.data();

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

            m_shared_memory.unlock();
        }
    }
    if (m_video_sink) m_video_sink->setVideoFrame(frame);
}