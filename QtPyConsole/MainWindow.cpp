#include "MainWindow.h"
#include "QtPyConsole.h"
#include "PyInteractiveWrapper.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), textEdit(new QtPyConsole(this)), interactiveConsole(new PyInteractiveWrapper)
{
    setWindowTitle("QtPyConsole");

    setCentralWidget(textEdit);

    textEdit->setPrefix("");
    textEdit->print(tr("Python %1 on %2").arg(Py_GetVersion()).arg(Py_GetPlatform()));
    textEdit->setPrefix(">>> ");
    textEdit->prepareCommandLine();

    connect(textEdit, SIGNAL(command(QString)), this, SLOT(handleCommand(QString)));
}

MainWindow::~MainWindow()
{
    delete interactiveConsole;
}

void MainWindow::handleCommand(QString cmd)
{
    //if (auto output = console->pyRun(cmd); !output.isEmpty()) textEdit->print(output);

    auto [isMultiLine, output] = interactiveConsole->push(cmd);
    if (isMultiLine)
    {
        textEdit->setPrefixColor(Qt::white);
        textEdit->setPrefix("... ");
    }
    else
    {
        textEdit->setPrefixColor(QColor(140, 255, 50));
        textEdit->setPrefix(">>> ");
    }
    textEdit->print(output);
    textEdit->prepareCommandLine();
}