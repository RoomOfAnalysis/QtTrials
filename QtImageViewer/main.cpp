#include <QApplication>

#include "ImageViewer.h"

#include <QImageReader>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    //QApplication::addLibraryPath(QApplication::applicationDirPath());
    //qDebug() << QApplication::libraryPaths();

    qDebug() << QImageReader::supportedImageFormats() << QImageReader::supportedMimeTypes();

    ImageViewer w;
    w.show();

    return a.exec();
}