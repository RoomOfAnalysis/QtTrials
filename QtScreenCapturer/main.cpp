#include <QtWidgets/QApplication>
#include <QDateTime>

#include <cstdio>
#include <cstdlib>

#include "Viewer.h"

// context only avaliable in Debug build
void QtMessageOutput(QtMsgType type, QMessageLogContext const& context, QString const& msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char* file = context.file ? context.file : "";
    const char* function = context.function ? context.function : "";
    switch (type)
    {
    case QtDebugMsg:
        fprintf(stderr, "%s - [Debug]: %s (%s:%u, %s)\n",
                QDateTime::currentDateTime().toString(Qt::ISODate).toStdString().c_str(), localMsg.constData(), file,
                context.line, function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "%s - [Info]: %s (%s:%u, %s)\n",
                QDateTime::currentDateTime().toString(Qt::ISODate).toStdString().c_str(), localMsg.constData(), file,
                context.line, function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "%s - [Warning]: %s (%s:%u, %s)\n",
                QDateTime::currentDateTime().toString(Qt::ISODate).toStdString().c_str(), localMsg.constData(), file,
                context.line, function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "%s - [Critical]: %s (%s:%u, %s)\n",
                QDateTime::currentDateTime().toString(Qt::ISODate).toStdString().c_str(), localMsg.constData(), file,
                context.line, function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s - [Fatal]: %s (%s:%u, %s)\n",
                QDateTime::currentDateTime().toString(Qt::ISODate).toStdString().c_str(), localMsg.constData(), file,
                context.line, function);
        break;
    }
}

int main(int argc, char* argv[])
{
    qInstallMessageHandler(QtMessageOutput);
    QApplication a(argc, argv);
    Viewer v;
    v.show();
    return a.exec();
}
