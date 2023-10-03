#include <QtWidgets/QApplication>
#include <QMessageBox>

#include "SingleAppGuard.h"

int main(int argc, char* argv[])
{
    // only one instance is allowed
    SingleAppGuard guard("LungViewer");
    if (!guard.tryRun())
    {
        QApplication _(argc, argv);
        QMessageBox msgBox;
        msgBox.setText("The app already launched!");
        return msgBox.exec();
    }

    QApplication a(argc, argv);
    return a.exec();
}
