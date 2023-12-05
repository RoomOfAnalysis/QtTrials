#include "QtOverlay.h"

#include <QApplication>
#include <QPainter>
#include <QLabel>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QLabel w;
    w.setText("test");

    QtOverlayFactoryFilter f([](QPainter* painter) { painter->fillRect(painter->window(), {80, 80, 255, 128}); }, &w);
    w.installEventFilter(&f);

    w.show();

    return a.exec();
}