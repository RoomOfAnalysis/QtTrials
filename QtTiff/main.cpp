#include <QApplication>
#include <QFileDialog>
#include <QImage>
#include <QImageReader>
#include <QColorSpace>
#include <QDebug>
#include <QLabel>
#include <QThread>

//#include <tiff.h>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    //qDebug() << TIFF_VERSION_BIG;

    auto label = new QLabel();
    label->resize(500, 500);
    if (auto path = QFileDialog::getOpenFileName(nullptr, "Open Image", QDir::homePath()); !path.isEmpty())
    {
        // QImage img;
        // qDebug() << img.load(path);
        // qDebug() << img.size();
        // qDebug() << img.colorSpace().iccProfile();

        qDebug() << QImageReader::supportedImageFormats();
        QImageReader reader(path);
        qDebug() << reader.imageCount();
        label->show();
        for (auto i = 0; i < reader.imageCount(); i++)
        {
            auto img = reader.read();
            //qDebug() << i << img.size();
            label->setPixmap(QPixmap::fromImage(img).scaled(500, 500, Qt::KeepAspectRatio));
            if (!reader.jumpToNextImage()) break;

            QApplication::processEvents();
            QThread::sleep(1); // 1s
        }
    }

    return app.exec();
}