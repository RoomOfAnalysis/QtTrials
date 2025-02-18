#include <QApplication>
#include <QMainWindow>
#include <QSplitter>
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>

#include "QtImageComparatorSlider.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto mw = new QMainWindow();

    auto img1 = new QPixmap(argv[1]);
    auto img2 = new QPixmap(argv[2]);

    auto w = new QtImageComparatorSlider;
    w->setStyleSheet("QSplitter{background-color:black;}; QSplitter::handle{background-color: rgb(255, 255, 255);}");
    w->setPixmap1(*img1);
    w->setPixmap2(*img2);
    //w->setOrientation(Qt::Orientation::Vertical);
    mw->setCentralWidget(w);
    mw->show();

    return app.exec();
}