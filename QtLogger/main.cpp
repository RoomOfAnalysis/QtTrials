#include <QCoreApplication>
#include <QDateTime>

#include <cstdio>
#include <cstdlib>

// need define this macro
// or in cmakelist add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include <spdlog/spdlog.h>

// use fprintf
// context only avaliable in Debug build
// https://doc.qt.io/qt-6/qtlogging.html
static void MsgHandlerFprintf(QtMsgType type, QMessageLogContext const& context, QString const& msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const std::string timestamp = QDateTime::currentDateTime().toString(Qt::ISODate).toStdString();
    switch (type)
    {
    case QtDebugMsg:
        fprintf(stderr, "%s - [Debug]: %s (%s:%u, %s)\n", timestamp.c_str(), localMsg.constData(), context.file,
                context.line, context.function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "%s - [Info]: %s (%s:%u, %s)\n", timestamp.c_str(), localMsg.constData(), context.file,
                context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "%s - [Warning]: %s (%s:%u, %s)\n", timestamp.c_str(), localMsg.constData(), context.file,
                context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "%s - [Critical]: %s (%s:%u, %s)\n", timestamp.c_str(), localMsg.constData(), context.file,
                context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s - [Fatal]: %s (%s:%u, %s)\n", timestamp.c_str(), localMsg.constData(), context.file,
                context.line, context.function);
        break;
    }
}

// use spdlog
// use qt context and spdlog function
static void MsgHandlerSpdlog(QtMsgType type, QMessageLogContext const& context, QString const& msg)
{
    QByteArray localMsg = context.file ? QString("%1 (%2:L%3, %4)")
                                             .arg(msg, context.file, QString::number(context.line), context.function)
                                             .toLocal8Bit() :
                                         msg.toLocal8Bit();
    switch (type)
    {
    case QtDebugMsg:
        spdlog::debug(localMsg.constData());
        break;
    case QtInfoMsg:
        spdlog::info(localMsg.constData());
        break;
    case QtWarningMsg:
        spdlog::warn(localMsg.constData());
        break;
    case QtCriticalMsg:
        spdlog::critical(localMsg.constData());
        break;
    case QtFatalMsg:
        spdlog::error(localMsg.constData());
        break;
    }
}
// use spdlog macro
static void MsgHandlerSpdlogMacro(QtMsgType type, QMessageLogContext const& context, QString const& msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type)
    {
    case QtDebugMsg:
        SPDLOG_DEBUG(localMsg.constData());
        break;
    case QtInfoMsg:
        SPDLOG_INFO(localMsg.constData());
        break;
    case QtWarningMsg:
        SPDLOG_WARN(localMsg.constData());
        break;
    case QtCriticalMsg:
        SPDLOG_CRITICAL(localMsg.constData());
        break;
    case QtFatalMsg:
        SPDLOG_ERROR(localMsg.constData());
        break;
    }
}

int main(int argc, char* argv[])
{
    //qInstallMessageHandler(MsgHandlerFprintf);

    //spdlog::set_level(spdlog::level::debug);
    //// https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
    //spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e][thread %t][%l] : %v");
    //qInstallMessageHandler(MsgHandlerSpdlog);

    spdlog::set_level(spdlog::level::debug);
    // https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e][thread %t][%l] : %v (%@, %!)");
    qInstallMessageHandler(MsgHandlerSpdlogMacro);

    QCoreApplication app(argc, argv);

    qDebug() << "this is a debug message";

    return app.exec();
}