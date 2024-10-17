#include "MainWindow.h"
#include "QtPyConsole.h"
#include "PyWrapper.h"

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent), textEdit(new QtPyConsole(this)), console(new PyWrapper)
{
    setWindowTitle("QtPyConsole");

    setCentralWidget(textEdit);

    connect(textEdit, SIGNAL(command(QString)), this, SLOT(handleCommand(QString)));
}

MainWindow::~MainWindow()
{
    delete console;
}

void MainWindow::handleCommand(QString cmd)
{
    if (auto output = console->pyRun(cmd); !output.isEmpty()) textEdit->print(output);
    textEdit->prepareCommandLine();
}