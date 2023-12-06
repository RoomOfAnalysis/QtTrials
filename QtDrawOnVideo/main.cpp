#include "QtCamera.h"
#include "QtDisplayBoard.h"

#include <QApplication>
#include <QTimer>
#include <QRandomGenerator>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QtDisplayBoard w;
    w.getVideoItem()->setSize({1920, 1080});

    QtCamera cam;
    cam.setCameraDevice(QtCamera::getAvailableCameras()[0]);
    cam.setVideoSink(w.getVideoItem()->videoSink());
    cam.start();

    w.showMaximized();

    QTimer timer;
    timer.setInterval(1000);
    // https://doc.qt.io/qt-5/graphicsview.html#the-graphics-view-coordinate-system
    QObject::connect(&timer, &QTimer::timeout, [&w]() {
        auto [width, height] = w.getVideoItem()->size().toSize();
        // FIXME: draw beyond video item boundary will cause the video item position change...
        auto x = QRandomGenerator::global()->bounded(50, width - 50);
        auto y = QRandomGenerator::global()->bounded(50, height - 80);
        w.getScene()->addEllipse(x, y, 30, 30, QPen(Qt::green, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin),
                                 QBrush(Qt::transparent));
        //qDebug() << width << height << x << y;
    });
    timer.start();

    return a.exec();
}