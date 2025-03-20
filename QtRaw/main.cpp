#include <QApplication>

#include "RawIOHandler.hpp"
#include <QFile>
#include <QFileDevice>
#include <QLabel>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    RawIOHandler handler;
    auto file = QFile(argv[1]);
    qDebug() << file.open(QIODevice::ReadOnly);
    handler.setDevice(&file);
    qDebug() << handler.canRead();

    auto thumnailSize = handler.thumbnailSize();
    auto fullSize = handler.fullSize();
    qDebug() << "Thumbnail Size:" << thumnailSize << "; Full Size:" << fullSize;

    // read thumbnail only
    handler.setOption(QImageIOHandler::ImageOption::ScaledSize, thumnailSize);

    QImage img;
    qDebug() << handler.read(&img);

    QLabel label;
    label.resize(thumnailSize);
    label.setPixmap(QPixmap::fromImage(img).scaled(label.size()));
    label.show();

    return app.exec();
}