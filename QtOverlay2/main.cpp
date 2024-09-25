
#include "../QtImageViewer/Image.h"
#include "QtImageViewer.h"
#include "QtImageInfoOverlay.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        qDebug() << "Required input image path, e.g. xxx.jpg";
        return -1;
    }

    QApplication a(argc, argv);

    Image img(argv[1]);
    QtImageViewer w;
    w.setStyleSheet("background-color:white");
    QtImageInfoOverlay overlay(&w);
    img.metadata()->loadExifTags();
    overlay.setExifInfo(img.metadata()->getExifTags());
    w.show();
    w.showImage(img.pixmap());

    return a.exec();
}