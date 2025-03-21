#include <QApplication>

#include "RawIOHandler.hpp"
#include "RawReader.hpp"

#include <QFile>
#include <QFileDevice>
#include <QLabel>

static const QSet<QString> raw_keys = {"arw", "crw", "cr2", "dng",  "nef", "raf", "kderaw", "dcr", "k25", "kdc", "mrw",
                                       "nrw", "orf", "raw", "raw2", "rw",  "rw2", "pef",    "srw", "x3f", "sr2", "srf"};

QImage loadRawImage(QString const& path)
{
    auto file = QFile(path);
    if (raw_keys.contains(file.fileName().split(".").back()))
    {
        RawReader reader;
        if (file.open(QIODeviceBase::ReadOnly) && reader.setDevice(&file)) return reader.read(reader.thumbnailSize());
    }
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

    auto img = loadRawImage(argv[1]);

    QLabel label;
    label.resize(img.size());
    label.setPixmap(QPixmap::fromImage(img));
    label.show();

    return app.exec();
}