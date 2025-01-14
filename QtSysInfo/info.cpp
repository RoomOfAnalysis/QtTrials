#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QSysInfo>
#include <QProcess>
#include <QTimer>
#include <QDebug>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QMainWindow window;
    window.resize(400, 300);

    QLabel* label = new QLabel(&window);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    window.setCentralWidget(label);

    // memory info
    QString memoryInfo = "Memory Information:\n";

    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    if (GlobalMemoryStatusEx(&memoryStatus))
    {
        memoryInfo +=
            QString("Total Physical Memory: %1 %2\n").arg(memoryStatus.ullTotalPhys / (1024 * 1024)).arg("MB");
        memoryInfo +=
            QString("Available Physical Memory: %1 %2\n").arg(memoryStatus.ullAvailPhys / (1024 * 1024)).arg("MB");
        memoryInfo +=
            QString("Total Virtual Memory: %1 %2\n").arg(memoryStatus.ullTotalVirtual / (1024 * 1024)).arg("MB");
        memoryInfo +=
            QString("Available Virtual Memory: %1 %2\n").arg(memoryStatus.ullAvailVirtual / (1024 * 1024)).arg("MB");
    }
    else
        memoryInfo += QString("failed to obtain memory status\n");

    //cpu info
    QString cpuInfo = "CPU Information:\n";
    QProcess cpuProcess;
    cpuProcess.start("wmic", {"cpu", "get", "Name"});
    cpuProcess.waitForFinished();
    QString cpuResult = cpuProcess.readAllStandardOutput();
    QString cpuName = cpuResult.split("\n").at(1).trimmed();
    cpuInfo += "Model: " + cpuName + "\n";

    // gpu info
    QString gpuInfo = "GPU Information:\n";
    QProcess gpuProcess;
    gpuProcess.start("wmic", {"path", "win32_VideoController", "get", "Name"});
    gpuProcess.waitForFinished();
    QString gpuResult = gpuProcess.readAllStandardOutput();
    QStringList gpuList = gpuResult.split("\n", Qt::SkipEmptyParts);
    for (int i = 1; i < gpuList.size(); i++)
    {
        QString gpuName = gpuList.at(i).trimmed();
        if (!gpuName.isEmpty()) gpuInfo += "GPU " + QString::number(i) + ": " + gpuName + "\n";
    }

    QString systemInfo = memoryInfo + "\n" + cpuInfo + "\n" + gpuInfo;

    auto* timer = new QTimer();
    timer->setInterval(1000);

    // real-time gpu usage by `nvidia-smi`
    auto* nvi_process = new QProcess();
    QObject::connect(timer, &QTimer::timeout, [nvi_process, label, systemInfo]() {
        nvi_process->start("nvidia-smi", {"--query-gpu=utilization.gpu,utilization.memory", "--format=csv"});
        nvi_process->waitForFinished();
        QString gpuResult = nvi_process->readAllStandardOutput();
        auto r = gpuResult.split("\n", Qt::SkipEmptyParts).last().split(",", Qt::SkipEmptyParts);
        if (r.size() == 2)
            label->setText(systemInfo + "\n" +
                           QString("GPU Usage: %1, GPU Memory Usage: %2").arg(r.first()).arg(r.last()));
    });
    timer->start();

    label->setText(systemInfo);

    window.show();

    return a.exec();
}
