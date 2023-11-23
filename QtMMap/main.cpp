#include <QApplication>

#include "Viewer.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    Viewer v;
    v.show();
    v.start();

    return a.exec();
}