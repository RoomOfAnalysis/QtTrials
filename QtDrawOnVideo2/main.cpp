#include <QApplication>
#include <QLabel>
#include <QTimer>
#include <QRandomGenerator>

#include "FrameWriter.h"
#include "QtCamera.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    auto* iv = new ImageViewer();
    FrameWriter fw;
    fw.set_image_viewer(iv);

    QtCamera cam;
    cam.setCameraDevice(QtCamera::getAvailableCameras()[0]);
    cam.setVideoSink(&fw);
    cam.start();

    QWidget w;
    iv->setParent(&w);
    w.show();

    QTimer timer;
    timer.setInterval(1000);
    // https://doc.qt.io/qt-5/graphicsview.html#the-graphics-view-coordinate-system
    QObject::connect(&timer, &QTimer::timeout, [iv]() {
        iv->disable_draw();
        auto [width, height] = iv->size();
        auto x = QRandomGenerator::global()->bounded(50, width - 50);
        auto y = QRandomGenerator::global()->bounded(50, height - 80);
        iv->drawCircle(x, y, 30, 30);
    });
    timer.start();

    return a.exec();
}