#include <QApplication>

#include "../QtImageViewer/Image.h"
#include "ImageViewer.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    Image img(argv[1]);
    ImageViewer viewer;
    viewer.loadImage(img.image());
    viewer.show();

    return a.exec();
}