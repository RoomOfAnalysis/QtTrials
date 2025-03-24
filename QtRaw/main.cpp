#include <QApplication>
#include <QFile>
#include <QFileDevice>
#include <QLabel>
#include <QElapsedTimer>

//#define USE_KIMAGE

#ifndef USE_KIMAGE
#include "RawIOHandler.hpp"
#include "RawReader.hpp"
static const QSet<QString> raw_keys = {"arw", "crw", "cr2", "dng",  "nef", "raf", "kderaw", "dcr", "k25", "kdc", "mrw",
                                       "nrw", "orf", "raw", "raw2", "rw",  "rw2", "pef",    "srw", "x3f", "sr2", "srf"};
#else
#include <QImageReader>
#endif // ! USE_KIMAGE

QImage loadRawImage(QString const& path)
{
#ifndef USE_KIMAGE
    auto file = QFile(path);
    if (raw_keys.contains(file.fileName().split(".").back()))
    {
        RawReader reader;
        if (file.open(QIODeviceBase::ReadOnly) && reader.setDevice(&file)) return reader.read(reader.thumbnailSize());
    }
#else
    // https://github.com/KDE/kimageformats/blob/master/src/imageformats/raw_p.h
    // try to set quality to zero to only load thumbnail but still very slow...
    // if use the same `LoadTHUMB` implementation with [kimageformats](https://github.com/KDE/kimageformats/blob/master/src/imageformats/raw.cpp#L582)
    // the time consumption should be 50 ms (kimage) vs 20 ms, it seems like setting quality to zero not working...
    QImageReader reader;
    // qDebug() << reader.supportedImageFormats();
    reader.setFileName(path);
    reader.setQuality(0);
    QImage img;
    if (reader.read(&img)) return img;
#endif // ! USE_KIMAGE

    return QImage(path);
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // RawIOHandler handler;
    // auto file = QFile(argv[1]);
    // qDebug() << file.open(QIODevice::ReadOnly);
    // handler.setDevice(&file);
    // qDebug() << handler.canRead();

    // auto thumnailSize = handler.thumbnailSize();
    // auto fullSize = handler.fullSize();
    // qDebug() << "Thumbnail Size:" << thumnailSize << "; Full Size:" << fullSize;

    // // read thumbnail only
    // handler.setOption(QImageIOHandler::ImageOption::ScaledSize, thumnailSize);

    // QImage img;
    // qDebug() << handler.read(&img);

    QElapsedTimer timer;
    timer.start();

    auto img = loadRawImage(argv[1]);

    qDebug() << timer.elapsed() << "ms"; // 20 ms vs 600 ms ...

    QLabel label;
    label.resize(img.size());
    label.setPixmap(QPixmap::fromImage(img));
    label.show();

    return app.exec();
}