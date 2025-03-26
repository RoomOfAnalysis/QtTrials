#include <QApplication>
#include "QtRhiViewer.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto viewer = QScopedPointer(new QtRhiViewer);
    viewer->show(); // show first to initialize gpu resources
    viewer->setImage(QImage(argv[1]));

    return app.exec();
}