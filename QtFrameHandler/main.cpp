#include "QtCamera.h"
#include "QtFrameHandler.h"

#include <QApplication>
#include <QVideoWidget>
#include <QPainter>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QVideoWidget w;
    QtFrameHandler h;
    h.setVideoSink(w.videoSink());

    auto processor = [](QImage* img) {
        QPainter painter(img);
        painter.setRenderHints(QPainter::LosslessImageRendering);
        painter.fillRect(0, 0, 500, 500, Qt::red);
    };
    h.setFrameProcessor(processor);

    QtCamera cam;
    cam.setCameraDevice(QtCamera::getAvailableCameras()[0]);
    cam.setVideoSink(&h);
    cam.start();

    w.show();

    return a.exec();
}