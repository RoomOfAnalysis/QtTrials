#include <QApplication>

#include "MainWindow.h"

// TODO: invesigate paraview's python shell
// https://github.com/Kitware/ParaView/blob/master/Qt/Python/pqPythonShell.h
// https://github.com/Kitware/VTK/blob/master/Utilities/PythonInterpreter/vtkPythonInteractiveInterpreter.h

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // to handle the warnning `Could not find platform independent libraries <prefix>`
    // https://docs.python.org/3/c-api/init_config.html#init-path-config
    // need to copy both `Lib` and `DLLs` folders from AppData/Local/Programs/Python/Pyhont311 to this application dir
    // otherwise it will use system's python folder (searching path can be found in `sys.path`)
    // TODO: copy those folders in CMake post build command?
    if (qgetenv("PYTHONPATH").isEmpty())
    {
        auto cwd = QApplication::applicationDirPath().toUtf8();
        qputenv("PYTHONPATH", cwd);
        qputenv("PYTHONHOME", cwd);
    }

    MainWindow w;
    w.resize(800, 600);
    w.show();

    return app.exec();
}