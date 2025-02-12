#include <QApplication>
#include <Q3DScatterWidgetItem>
#include <QQuickWidget>
#include <QQmlComponent>

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    //qputenv("PATH", qgetenv("PATH").append("D:\\vcpkg\\installed\\x64-windows\\tools\\Qt6\\bin"));

    auto w = new MainWindow();
    w->show();

    return app.exec();
}

// FIXME: always failed to load mesh...
// Mesh file invalid
// QSSG.warning: Failed to load mesh: xxxx

// .\balsam.exe xxx.obj
// Failed to import file with error: unsupported file extension obj
// tried .ply / .obj / .fbx / .blend, all failed with balsam...

// Dump of file D:\vcpkg\installed\x64-windows\tools\Qt6\bin\balsam.exe

// File Type: EXECUTABLE IMAGE

//   Image has the following dependencies:

//     Qt6Quick3DAssetImport.dll
//     Qt6Quick3DIblBaker.dll
//     Qt6Gui.dll
//     Qt6Core.dll
//     ...

// Dump of file D:\vcpkg\installed\x64-windows\debug\Qt6\plugins\sceneparsers\assimpsceneimportd.dll

// File Type: DLL

//   Image has the following dependencies:

//     Qt63DAnimationd.dll
//     Qt63DExtrasd.dll
//     Qt63DRenderd.dll
//     Qt63DCored.dll
//     Qt6Guid.dll
//     Qt6Cored.dll
//     assimp-vc143-mtd.dll
//     ...

// Dump of file D:\vcpkg\installed\x64-windows\debug\bin\assimp-vc143-mtd.dll

// File Type: DLL

//   Image has the following dependencies:

//     poly2tri.dll
//     minizip.dll
//     zlibd1.dll
//     pugixml.dll
//     ...
