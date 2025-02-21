#include "QtUtils.h"

#include <QApplication>
#include <QMainWindow>
#include <QLayout>
#include <QPushButton>
#include <QLabel>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QObject::connect(qApp, &QApplication::aboutToQuit, []() { qDebug() << "app quit"; });

    auto* btn = new QPushButton("reboot");
    QObject::connect(btn, &QPushButton::pressed, []() { QtUtils::rebootApp(); });

    auto* label = new QLabel;

    auto* layout = new QVBoxLayout;
    layout->addWidget(btn, 0);
    layout->addWidget(label, 1);

    auto* w = new QWidget();
    w->setLayout(layout);
    w->setFixedSize(1000, 1000);
    w->show();

    qDebug() << QtUtils::systemInfo();

    qDebug() << QtUtils::desktopRect();
    label->setPixmap(QtUtils::grabCombinedScreens().scaled(label->size(), Qt::KeepAspectRatio));

    qDebug() << QtUtils::getPidByProcessName(QApplication::applicationName() + ".exe")
             << QApplication::applicationPid();

    w->move(0, 0);
    QThread::currentThread()->msleep(1000);
    QtUtils::moveToCenter(w);

    return app.exec();
}