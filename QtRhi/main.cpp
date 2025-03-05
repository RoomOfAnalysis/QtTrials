#include <QApplication>

#include "QtRhiViewer.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto v = QScopedPointer(new QtRhiViewer);
    v->textureMapToPlane(true);
    v->loadMesh(argv[1]);
    v->loadTexture(argv[2]);
    v->show();

    return app.exec();
}