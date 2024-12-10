#include <QApplication>
#include <QLineSeries>
#include <QValueAxis>
#include <QChart>
#include <QChartView>
#include <QMainWindow>

#include "dataqueryengine.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto pid = QApplication::applicationPid();

    auto* dq = new DataQueryEngine();
    dq->setPid(pid);
    dq->setTimeInterval(1000);

    static int cnt = 0;

    auto cpuSeries = new QLineSeries();

    QObject::connect(dq, &DataQueryEngine::cpuDataUpdated,
                     [cpuSeries](int percent) { cpuSeries->append(cnt++, percent); });

    auto axisX = new QValueAxis;
    axisX->setRange(0, 60);
    axisX->setLabelFormat("%d");
    axisX->setTitleText("s");
    auto axisY = new QValueAxis;
    axisY->setLabelFormat("%.1f");
    axisY->setRange(0, 100);
    axisY->setTitleText("Percentage");

    auto chart = new QChart;
    chart->setTitle("CPU");
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    chart->addSeries(cpuSeries);

    cpuSeries->attachAxis(axisX);
    cpuSeries->attachAxis(axisY);

    cpuSeries->setColor(Qt::red);

    auto* chartView = new QChartView(chart);
    auto* w = new QMainWindow();
    w->setCentralWidget(chartView);
    w->show();

    dq->startDataQuery();

    return app.exec();
}