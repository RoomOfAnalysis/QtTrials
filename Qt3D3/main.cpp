#include <QApplication>
#include <QScopedPointer>

#include "QtGLWidget.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto w = QScopedPointer(new QtGLWidget);
    w->show();
    // QOpenGLContext is initialized after show
    w->loadMesh(argv[1]);

    return app.exec();
}