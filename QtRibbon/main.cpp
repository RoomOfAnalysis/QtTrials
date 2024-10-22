// code from https://github.com/czyt1988/SARibbon/tree/master/example/MainWindowExample

#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>
#include "SARibbonBar.h"

void log_out_put(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QByteArray localMsg = msg.toLocal8Bit();

    switch (type)
    {
    case QtDebugMsg:
        fprintf(stdout, "%s |[Debug] (%s[%u],%s)\n", localMsg.constData(), context.function, context.line,
                context.file);
        break;

    case QtWarningMsg:
        fprintf(stdout, "%s |[Warning] (%s[%u],%s)\n", localMsg.constData(), context.function, context.line,
                context.file);
        break;

    case QtCriticalMsg:
        fprintf(stdout, "%s |[Critical] (%s[%u],%s)\n", localMsg.constData(), context.function, context.line,
                context.file);
        break;

    case QtFatalMsg:
        fprintf(stdout, "%s |[Fatal] (%s[%u],%s)\n", localMsg.constData(), context.function, context.line,
                context.file);
        abort();
        break;

    default:
        fprintf(stdout, "%s |[Debug](%s[%u],%s)\n", localMsg.constData(), context.function, context.line, context.file);
        break;
    }
#ifndef QT_NO_DEBUG_OUTPUT
    fflush(stdout);
#endif
}

int main(int argc, char* argv[])
{
    SARibbonBar::initHighDpi();

    QApplication a(argc, argv);
    qInstallMessageHandler(log_out_put);
    QFont f = a.font();
    f.setFamily(u8"微软雅黑");
    a.setFont(f);
    QElapsedTimer cost;

    cost.start();
    MainWindow w;
    qDebug() << "window build cost:" << cost.elapsed() << " ms";
    w.show();

    return (a.exec());
}
