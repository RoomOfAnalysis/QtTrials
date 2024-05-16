#include "QtAspectWidget.h"

#include <QApplication>
#include <QGridLayout>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    auto* w = new QWidget();
    w->setStyleSheet("background: red;border: 5px solid blue");

    auto* layout = new QGridLayout();
    layout->addWidget(w);

    QtAspectWidget m;
    m.setStyleSheet("background: yellow;");
    m.setLayout(layout);

    m.show();

    return a.exec();
}