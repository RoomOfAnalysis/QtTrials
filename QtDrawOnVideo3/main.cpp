#include <QApplication>
#include <QLabel>
#include <QTimer>
#include <QRandomGenerator>
#include <QGraphicsView>

#include "FrameWriter.h"
#include "QtCamera.h"
#include "QtDisplayBoard.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    FrameWriter fw;

    QtCamera cam;
    cam.setCameraDevice(QtCamera::getAvailableCameras()[0]);
    cam.setVideoSink(&fw);
    cam.start();

    static bool first_img = true;
    QtDisplayBoard board;
    QObject::connect(&fw, &FrameWriter::frameReady, [&board](QImage img) {
        board.getPixmapItem()->setPixmap(QPixmap::fromImage(img));
        if (first_img && !board.getScene()->views().isEmpty())
        {
            auto* view = board.getScene()->views()[0];
            view->fitInView(board.getPixmapItem());
            view->centerOn(board.getPixmapItem());
            first_img = false;
        }
    });
    board.show();

    QTimer timer;
    timer.setInterval(1000);
    // https://doc.qt.io/qt-5/graphicsview.html#the-graphics-view-coordinate-system
    QObject::connect(&timer, &QTimer::timeout, [&board]() {
        auto [width, height] = board.size();
        auto x = QRandomGenerator::global()->bounded(50, width - 50);
        auto y = QRandomGenerator::global()->bounded(50, height - 80);
        board.getScene()->addEllipse(x, y, 30, 30,
                                     QPen(QColor(255, 255, 255, 255), 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin),
                                     QBrush(Qt::transparent));
    });
    timer.start();

    return a.exec();
}