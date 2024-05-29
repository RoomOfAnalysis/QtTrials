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
            //view->centerOn(board.getPixmapItem());
            // centerOn may not correctly center the item
            // according to https://forum.qt.io/topic/51541/centering-an-item-in-a-qgraphicsview/2
            // setSceneRect will solve the problem
            board.getScene()->setSceneRect(board.getPixmapItem()->sceneBoundingRect());
            first_img = false;
        }
    });
    board.show();

    QTimer timer;
    timer.setInterval(1000);
    // https://doc.qt.io/qt-5/graphicsview.html#the-graphics-view-coordinate-system
    QObject::connect(&timer, &QTimer::timeout, [&board]() {
        for (auto item : board.getScene()->items())
            if (item != board.getPixmapItem()) board.getScene()->removeItem(item);
        auto [width, height] = board.getPixmapItem()->boundingRect().size().toSize();
        int xx[] = {0, width};
        int yy[] = {0, height};
        auto x = xx[QRandomGenerator::global()->bounded(0, 2)];
        auto y = yy[QRandomGenerator::global()->bounded(0, 2)];
        // save and restore scene_rect to make sure adding item not moves the scene
        auto scene_rect = board.getScene()->sceneRect();
        board.getScene()->addEllipse(x, y, 30, 30, QPen(Qt::green, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin),
                                     QBrush(Qt::transparent));
        auto* txt_item = board.getScene()->addText("hello world!");
        txt_item->setDefaultTextColor(Qt::yellow);
        txt_item->moveBy(x, y);
        board.getScene()->setSceneRect(scene_rect);
    });
    timer.start();

    return a.exec();
}