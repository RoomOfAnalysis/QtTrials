#include "ZMQClient.h"

#include <QApplication>
#include <QTimer>
#include <QProcess>
#include <QDebug>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    bool is_windows = (QSysInfo::productType() == "windows");
    QProcess server_process;
    server_process.setProgram(is_windows ? "cmd.exe" : "bash");
    server_process.setArguments({{(is_windows ? "/C" : "-c")}, "python ../../QtZMQ/server.py"});
    server_process.setProcessChannelMode(QProcess::SeparateChannels);
    QObject::connect(&server_process, &QProcess::readyReadStandardError,
                     [&server_process]() { qDebug() << server_process.readAllStandardError(); });
    QObject::connect(&server_process, &QProcess::readyReadStandardOutput,
                     [&server_process]() { qDebug() << server_process.readAllStandardOutput(); });
    QObject::connect(&server_process, &QProcess::errorOccurred, [&server_process]() {
        qCritical() << "algo server error occurred:" << server_process.errorString();
    });
    server_process.start();

    ZMQClient client{};
    client.set_address("tcp://localhost:5570");
    QObject::connect(&client, &ZMQClient::ready_to_read, [&]() { qDebug() << client.receive()[0]; });
    QTimer timer(&client);
    int cnt = 0;
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        auto sent = client.send({"hello"});
        assert(sent);

        if (cnt++ == 10)
        {
            timer.stop();
            server_process.terminate();
            if (!server_process.waitForFinished(2000)) server_process.kill();
            server_process.disconnect();
            server_process.deleteLater();
            app.quit();
        }
    });
    timer.start(100);

    return app.exec();
}
